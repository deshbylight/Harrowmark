// JamProbe: unattended evidence for agents.
//   -JamShots=<dir> [-JamShotSeconds=N] [-JamShotCount=K] [-JamExecFile=<file>]
// Waits until the game world has begun play and shader/asset compilation and async loading are
// done, then runs N seconds: K screenshots (shot_00.png ...) spread over the run, console commands
// from the exec file ("<seconds> <command>" per line, seconds counted from ready), per-frame
// timing collected into <dir>/perf.json (frames around screenshots and commands excluded).
// Exits once every screenshot file exists.
// Used by `jf shot`; the agent then opens the PNGs and reads perf.json.
// -JamVideo (used by `jf video`): no screenshots; from ready until the end it records real-time
// frames (<dir>/frames/frame_NNNNN.jpg, numbered by 30 fps time slot, so dropped slots leave gaps)
// and the main submix (<dir>/audio.wav); jf fills the gaps and encodes the mp4.

#include "CoreMinimal.h"
#include "AssetCompilingManager.h"
#include "AudioMixerBlueprintLibrary.h"
#include "Containers/Ticker.h"
#include "Engine/GameEngine.h"
#include "MovieSceneCapture.h"
#include "UObject/Package.h"
#include "DynamicRHI.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformTime.h"
#include "JamCore.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "RenderTimer.h"
#include "ShaderCompiler.h"
#include "UnrealClient.h"

class FJamProbeModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		if (GIsEditor || IsRunningCommandlet() || !FParse::Value(FCommandLine::Get(), TEXT("JamShots="), ShotDir))
		{
			return;
		}
		FParse::Value(FCommandLine::Get(), TEXT("JamShotSeconds="), TotalSeconds);
		FParse::Value(FCommandLine::Get(), TEXT("JamShotCount="), ShotCount);
		TotalSeconds = FMath::Max(TotalSeconds, 4.0);
		bVideo = FParse::Param(FCommandLine::Get(), TEXT("JamVideo"));
		ShotCount = bVideo ? 0 : FMath::Clamp(ShotCount, 1, 30);
		IFileManager::Get().MakeDirectory(*ShotDir, true);

		FString ExecFile;
		if (FParse::Value(FCommandLine::Get(), TEXT("JamExecFile="), ExecFile))
		{
			LoadExecFile(ExecFile);
		}
		UE_LOG(LogJam, Display, TEXT("[JamProbe] %d shots over %.0fs, %d timed commands, into %s"),
			ShotCount, TotalSeconds, Commands.Num(), *ShotDir);
		StartTime = FPlatformTime::Seconds();
		// Every frame: perf sampling needs per-frame deltas.
		TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FJamProbeModule::Tick), 0.0f);
	}

	virtual void ShutdownModule() override
	{
		if (TickHandle.IsValid())
		{
			FTSTicker::RemoveTicker(TickHandle);
			TickHandle.Reset();
		}
	}

private:
	struct FTimedCommand
	{
		double At = 0.0;
		FString Command;
		bool bDone = false;
	};

	void LoadExecFile(const FString& File)
	{
		TArray<FString> Lines;
		if (!FFileHelper::LoadFileToStringArray(Lines, *File))
		{
			UE_LOG(LogJam, Error, TEXT("[JamProbe] cannot read exec file %s"), *File);
			return;
		}
		for (const FString& Raw : Lines)
		{
			const FString Line = Raw.TrimStartAndEnd();
			FString Seconds, Command;
			if (Line.IsEmpty() || !Line.Split(TEXT(" "), &Seconds, &Command) || !Seconds.IsNumeric())
			{
				if (!Line.IsEmpty())
				{
					UE_LOG(LogJam, Error, TEXT("[JamProbe] bad exec line (want '<seconds> <command>'): %s"), *Line);
				}
				continue;
			}
			Commands.Add({FCString::Atod(*Seconds), Command.TrimStart(), false});
		}
	}

	static UWorld* FindGameWorld()
	{
		if (!GEngine)
		{
			return nullptr;
		}
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			UWorld* World = Context.World();
			if (World && Context.WorldType == EWorldType::Game && World->HasBegunPlay())
			{
				return World;
			}
		}
		return nullptr;
	}

	static bool IsSettled(FString& OutWaitingFor)
	{
		if (!FindGameWorld())
		{
			OutWaitingFor = TEXT("game world");
			return false;
		}
		if (IsAsyncLoading())
		{
			OutWaitingFor = TEXT("async loading");
			return false;
		}
		if (GShaderCompilingManager && GShaderCompilingManager->GetNumRemainingJobs() > 0)
		{
			OutWaitingFor = FString::Printf(TEXT("%d shader jobs"), GShaderCompilingManager->GetNumRemainingJobs());
			return false;
		}
		if (FAssetCompilingManager::Get().GetNumRemainingAssets() > 0)
		{
			OutWaitingFor = FString::Printf(TEXT("%d compiling assets"), FAssetCompilingManager::Get().GetNumRemainingAssets());
			return false;
		}
		return true;
	}

	bool Tick(float)
	{
		const double Now = FPlatformTime::Seconds();
		if (ReadyTime < 0.0)
		{
			FString WaitingFor;
			if (IsSettled(WaitingFor))
			{
				ReadyTime = Now;
				UE_LOG(LogJam, Display, TEXT("[JamProbe] ready after %.1fs"), Now - StartTime);
				if (bVideo)
				{
					StartVideo();
				}
			}
			else if (Now - StartTime > SettleTimeout)
			{
				// Not settled: still capture, but the run is reported as failed (error line) so nobody
				// mistakes "Preparing Shaders" frames for the game's look.
				UE_LOG(LogJam, Error, TEXT("[JamProbe] not settled after %.0fs (waiting for %s); capturing anyway"),
					SettleTimeout, *WaitingFor);
				ReadyTime = Now;
			}
			else
			{
				return true;
			}
		}

		const double Elapsed = Now - ReadyTime;
		RunCommands(Elapsed);
		SamplePerf(Elapsed);

		// First shot after a warm-up (auto exposure, streaming); the last one at the end of the run.
		// A single shot is taken at the end, so the whole run is sampled for perf.json.
		const double Span = FMath::Max(TotalSeconds - Warmup(), 0.0);
		const double NextAt = ShotCount > 1 ? Warmup() + Span * Taken / (ShotCount - 1) : TotalSeconds;
		if (Taken < ShotCount && Elapsed >= NextAt && !FScreenshotRequest::IsScreenshotRequested())
		{
			const FString File = ShotPath(Taken);
			FScreenshotRequest::RequestScreenshot(File, /*bInShowUI*/ true, /*bAddFilenameSuffix*/ false);
			UE_LOG(LogJam, Display, TEXT("[JamProbe] shot %d at %.1fs -> %s"), Taken, Elapsed, *File);
			++Taken;
			SkipPerfFrames = PerfFramesSkippedPerEvent; // readback + PNG encode stall is the probe's, not the game's
		}

		if (Taken == ShotCount && Elapsed >= TotalSeconds)
		{
			// Screenshots are written a frame or more after the request: exit once all files exist.
			if (FinishTime < 0.0)
			{
				FinishTime = Now;
			}
			const bool bAllWritten = AllShotsWritten();
			if (bAllWritten || Now - FinishTime > 10.0)
			{
				if (!bAllWritten)
				{
					UE_LOG(LogJam, Error, TEXT("[JamProbe] screenshots missing 10s after the last request"));
				}
				WritePerf();
				StopVideo();
				UE_LOG(LogJam, Display, TEXT("[JamProbe] done; exiting"));
				TickHandle.Reset();
				FPlatformMisc::RequestExit(false, TEXT("JamProbe"));
				return false;
			}
		}
		return true;
	}

	double Warmup() const { return FMath::Min(3.0, TotalSeconds * 0.25); }

	FString ShotPath(int32 Index) const
	{
		return FPaths::Combine(ShotDir, FString::Printf(TEXT("shot_%02d.png"), Index));
	}

	bool AllShotsWritten() const
	{
		for (int32 i = 0; i < ShotCount; ++i)
		{
			if (!IFileManager::Get().FileExists(*ShotPath(i)))
			{
				return false;
			}
		}
		return true;
	}

	void RunCommands(double Elapsed)
	{
		for (FTimedCommand& Cmd : Commands)
		{
			if (!Cmd.bDone && Elapsed >= Cmd.At)
			{
				Cmd.bDone = true;
				SkipPerfFrames = PerfFramesSkippedPerEvent; // cvar/stat toggles hitch once
#if UE_ALLOW_EXEC_COMMANDS
				// Same routing as the in-game console: local player -> viewport/engine -> player
				// controller, cheat manager, pawn, game mode. GEngine->Exec alone misses the latter.
				UWorld* World = FindGameWorld();
				ULocalPlayer* Player = GEngine && World ? GEngine->GetFirstGamePlayer(World) : nullptr;
				const bool bHandled = Player ? Player->Exec(World, *Cmd.Command, *GLog)
					: GEngine && GEngine->Exec(World, *Cmd.Command, *GLog);
				UE_LOG(LogJam, Display, TEXT("[JamProbe] exec at %.1fs: %s%s"), Elapsed, *Cmd.Command,
					bHandled ? TEXT("") : TEXT(" (not handled)"));
#else
				UE_LOG(LogJam, Error, TEXT("[JamProbe] exec at %.1fs skipped, this build has no console commands: %s"),
					Elapsed, *Cmd.Command);
#endif
			}
		}
	}

	void SamplePerf(double Elapsed)
	{
		if (Elapsed < Warmup())
		{
			return; // skip load hitches and exposure adaptation
		}
		if (SkipPerfFrames > 0)
		{
			--SkipPerfFrames;
			++ExcludedFrames;
			return;
		}
		FrameMs.Add(static_cast<float>(FApp::GetDeltaTime() * 1000.0));
		GameMs += FPlatformTime::ToMilliseconds(GGameThreadTime);
		RenderMs += FPlatformTime::ToMilliseconds(GRenderThreadTime);
		GpuMs += FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles());
		PeakUsedMB = FMath::Max(PeakUsedMB, static_cast<double>(FPlatformMemory::GetStats().UsedPhysical) / (1024.0 * 1024.0));
	}

	void WritePerf() const
	{
		const int32 N = FrameMs.Num();
		if (N == 0)
		{
			UE_LOG(LogJam, Error, TEXT("[JamProbe] no frames sampled; perf.json not written"));
			return;
		}
		TArray<float> Sorted = FrameMs;
		Sorted.Sort();
		double Sum = 0.0;
		int32 Hitches = 0;
		for (const float Ms : FrameMs)
		{
			Sum += Ms;
			Hitches += Ms > 50.0f ? 1 : 0;
		}
		auto Pct = [&Sorted, N](double P) { return Sorted[FMath::Clamp(static_cast<int32>(P * (N - 1)), 0, N - 1)]; };
		const double AvgMs = Sum / N;
		const FString Json = FString::Printf(
			TEXT("{\"frames\": %d, \"avg_fps\": %.1f, \"avg_ms\": %.2f, \"p50_ms\": %.2f, \"p95_ms\": %.2f, ")
			TEXT("\"p99_ms\": %.2f, \"max_ms\": %.2f, \"hitches_over_50ms\": %d, \"game_thread_ms\": %.2f, ")
			TEXT("\"render_thread_ms\": %.2f, \"gpu_ms\": %.2f, \"peak_used_mb\": %.0f, \"excluded_probe_frames\": %d}"),
			N, 1000.0 / AvgMs, AvgMs, Pct(0.50), Pct(0.95), Pct(0.99), Sorted.Last(), Hitches,
			GameMs / N, RenderMs / N, GpuMs / N, PeakUsedMB, ExcludedFrames);
		FFileHelper::SaveStringToFile(Json, *FPaths::Combine(ShotDir, TEXT("perf.json")));
		UE_LOG(LogJam, Display, TEXT("[JamProbe] perf %s"), *Json);
	}

	void StartVideo()
	{
		UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
		UWorld* World = FindGameWorld();
		if (!GameEngine || !GameEngine->SceneViewport.IsValid() || !World)
		{
			UE_LOG(LogJam, Error, TEXT("[JamProbe] video: no game viewport/world (run with -game or a packaged build)"));
			return;
		}
		// Initialize() reads these from the command line; jf passes -MovieFolder, -MovieName,
		// -MovieFormat=JPG, -MovieFrameRate=30 and -MovieCinematicMode=false (keeps HUD, player, input).
		// -MovieFrameRate only sets CustomFrameRate, but the real-time strategy Initialize() builds uses
		// Settings.FrameRate (24 by default), so frames were spaced at 24 fps; set it before Initialize().
		int32 Fps = 30;
		FParse::Value(FCommandLine::Get(), TEXT("-MovieFrameRate="), Fps);
		UMovieSceneCapture* Capture = NewObject<UMovieSceneCapture>(GetTransientPackage());
		Capture->Settings.FrameRate = FFrameRate(FMath::Clamp(Fps, 1, 120), 1);
		Capture->Initialize(GameEngine->SceneViewport);
		Capture->StartCapture(); // registers with FActiveMovieSceneCaptures, which keeps it alive and ticks it
		VideoCapture = Capture;
		// The capture mutes the game for its image-only pass; sound is recorded from the submix here instead.
		FApp::SetVolumeMultiplier(1.0f);
		FApp::SetUnfocusedVolumeMultiplier(1.0f);
		// A silent submix auto-disables and the recorder then gets no data; keep it rendering (silence included).
		if (IConsoleVariable* NeverDisable = IConsoleManager::Get().FindConsoleVariable(TEXT("au.NeverDisableSubmixes")))
		{
			NeverDisable->Set(1, ECVF_SetByCode);
		}
		else
		{
			UE_LOG(LogJam, Warning, TEXT("[JamProbe] video: cvar au.NeverDisableSubmixes not found, silent stretches may drop audio"));
		}
		UAudioMixerBlueprintLibrary::StartRecordingOutput(World, static_cast<float>(TotalSeconds));
		VideoStartWall = FPlatformTime::Seconds();
		VideoStartWorld = World->GetRealTimeSeconds();
		bAudioRecording = true;
		UE_LOG(LogJam, Display, TEXT("[JamProbe] video capture started (%s)"), VideoCapture.IsValid() ? TEXT("frames + audio") : TEXT("audio only"));
		if (!VideoCapture.IsValid())
		{
			UE_LOG(LogJam, Error, TEXT("[JamProbe] video: frame capture did not start"));
		}
	}

	void StopVideo()
	{
		if (bAudioRecording)
		{
			bAudioRecording = false;
			if (UWorld* World = FindGameWorld())
			{
				UAudioMixerBlueprintLibrary::StopRecordingOutput(World, EAudioRecordingExportType::WavFile, TEXT("audio"), ShotDir);
			}
			else
			{
				UE_LOG(LogJam, Error, TEXT("[JamProbe] video: world gone, audio not saved"));
			}
		}
		if (UMovieSceneCapture* Capture = VideoCapture.Get())
		{
			const UWorld* World = FindGameWorld();
			const double CaptureSeconds = Capture->GetMetrics().ElapsedSeconds;
			const int32 Frames = Capture->GetMetrics().Frame;
			Capture->Close(); // finalizes the image protocol (flushes pending frame writes)
			UE_LOG(LogJam, Display, TEXT("[JamProbe] video capture finished: capture=%.2fs frames=%d wall=%.2fs world_real=%.2fs"),
				CaptureSeconds, Frames, FPlatformTime::Seconds() - VideoStartWall,
				World ? World->GetRealTimeSeconds() - VideoStartWorld : -1.0);
		}
		VideoCapture.Reset();
	}

	FString ShotDir;
	bool bVideo = false;
	bool bAudioRecording = false;
	TWeakObjectPtr<UMovieSceneCapture> VideoCapture;
	double VideoStartWall = 0.0;
	double VideoStartWorld = 0.0;
	double TotalSeconds = 20.0;
	int32 ShotCount = 5;
	int32 Taken = 0;
	double StartTime = 0.0;
	double ReadyTime = -1.0;
	double FinishTime = -1.0;
	double SettleTimeout = 180.0;
	TArray<FTimedCommand> Commands;
	TArray<float> FrameMs;
	double GameMs = 0.0;
	double RenderMs = 0.0;
	double GpuMs = 0.0;
	double PeakUsedMB = 0.0;
	static constexpr int32 PerfFramesSkippedPerEvent = 3;
	int32 SkipPerfFrames = 0;
	int32 ExcludedFrames = 0;
	FTSTicker::FDelegateHandle TickHandle;
};

IMPLEMENT_MODULE(FJamProbeModule, JamProbe);
