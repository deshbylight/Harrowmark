#include "InvestigationSubsystem.h"

#include "CaseDefinition.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "InvestigationSettings.h"
#include "Misc/Paths.h"
#include "HAL/IConsoleManager.h"
#include "PhotoSubjectComponent.h"

#if !UE_BUILD_SHIPPING
// Debug commands for test runs (jf shot --exec): the drive and the end of the night without walking there.
static FAutoConsoleCommandWithWorld GDriveCommand(TEXT("hm.Drive"), TEXT("Drive between the scene and the agency office."),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		if (UInvestigationSubsystem* Inv = UInvestigationSubsystem::Get(World))
		{
			Inv->Drive();
		}
	}));
static FAutoConsoleCommandWithWorld GEndNightCommand(TEXT("hm.EndNight"), TEXT("End the night now (dawn, or the forced report on the last night)."),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		if (UInvestigationSubsystem* Inv = UInvestigationSubsystem::Get(World))
		{
			Inv->EndNight();
		}
	}));
#endif

void UInvestigationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

UInvestigationSubsystem* UInvestigationSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UInvestigationSubsystem>() : nullptr;
}

bool UInvestigationSubsystem::StartCase(UCaseDefinition* InCase)
{
	UCaseDefinition* NewCase = InCase;
	if (!NewCase)
	{
		NewCase = UInvestigationSettings::Get().Case.LoadSynchronous();
	}
	if (!NewCase)
	{
		FString Error;
		const FString Path = FPaths::Combine(FPaths::ProjectDir(), UInvestigationSettings::Get().FallbackJson);
		NewCase = UCaseDefinition::LoadFromFile(Path, Error, this);
		if (!NewCase)
		{
			UE_LOG(LogInvestigation, Error, TEXT("[Investigation] no case asset configured and %s failed: %s"), *Path, *Error);
			return false;
		}
	}
	if (!ensureMsgf(NewCase->IsLoaded(), TEXT("case %s did not parse"), *NewCase->GetPathName()))
	{
		return false;
	}
	Case = NewCase;
	FCaseSim::Start(*Case, State);
	Prints.Reset();
	BoardLayout.Reset();
	BlackThreadsFrom.Reset();
	BlackThreadsTo.Reset();
	UE_LOG(LogInvestigation, Display, TEXT("[Investigation] case %s started: night 1 %s, %d plates"), *Case->CaseId.ToString(),
		*GetClockText(), State.Film);
	OnCaseChanged.Broadcast();
	return true;
}

FString UInvestigationSubsystem::GetClockText() const
{
	return FCaseRules::Clock(State.Clock);
}

ECaseActionResult UInvestigationSubsystem::Talk(FName Topic)
{
	if (!IsReady())
	{
		return ECaseActionResult::Invalid;
	}
	const ECaseActionResult R = FCaseSim::Talk(*Case, State, Topic);
	if (R == ECaseActionResult::Ok)
	{
		const FCaseCard& Card = *Case->FindCard(Topic);
		const FCasePerson* Speaker = Case->FindPerson(Card.Speaker);
		OnSubtitle.Broadcast(Speaker ? Speaker->Name : FText::FromName(Card.Speaker), Card.Text);
		AfterAction({Topic});
	}
	return R;
}

TMap<FName, ECaseActionResult> UInvestigationSubsystem::TopicsFor(FName Speaker) const
{
	TMap<FName, ECaseActionResult> Out;
	if (Case)
	{
		for (FName Topic : Case->TopicsOf(Speaker))
		{
			Out.Add(Topic, FCaseSim::CanTalk(*Case, State, Topic));
		}
	}
	return Out;
}

ECaseActionResult UInvestigationSubsystem::Shoot(const TArray<FCaseShotHit>& Hits, UTexture* Image)
{
	if (!IsReady())
	{
		return ECaseActionResult::Invalid;
	}
	TArray<FName> Gained;
	const ECaseActionResult R = FCaseSim::Shoot(*Case, State, Hits, &Gained);
	if (R == ECaseActionResult::Ok)
	{
		FCasePrint& Print = Prints.AddDefaulted_GetRef();
		Print.Night = State.Night;
		Print.Clock = State.Clock;
		Print.Image = Image;
		for (const FCaseShotHit& Hit : Hits)
		{
			if (State.Taken.Contains(Hit.Shot))
			{
				Print.Hits.Add(Hit);
			}
		}
		AfterAction(Gained);
	}
	return R;
}

ECaseActionResult UInvestigationSubsystem::Develop()
{
	if (!IsReady())
	{
		return ECaseActionResult::Invalid;
	}
	TArray<FName> Gained;
	const ECaseActionResult R = FCaseSim::Develop(*Case, State, &Gained);
	if (R == ECaseActionResult::Ok)
	{
		AfterAction(Gained);
	}
	return R;
}

ECaseActionResult UInvestigationSubsystem::Drive()
{
	if (!IsReady())
	{
		return ECaseActionResult::Invalid;
	}
	const ECaseActionResult R = FCaseSim::Drive(*Case, State);
	if (R == ECaseActionResult::Ok)
	{
		OnDrive.Broadcast(State.bAtHub);
		AfterAction({});
	}
	return R;
}

ECaseActionResult UInvestigationSubsystem::Enlarge(FName Detail)
{
	if (!IsReady())
	{
		return ECaseActionResult::Invalid;
	}
	const ECaseActionResult R = FCaseSim::Enlarge(*Case, State, Detail);
	if (R == ECaseActionResult::Ok)
	{
		AfterAction({Detail});
	}
	return R;
}

ECaseRedResult UInvestigationSubsystem::Red(const FCaseThreadEnd& A, const FCaseThreadEnd& B)
{
	if (!IsReady())
	{
		return ECaseRedResult::Invalid;
	}
	FName Stamped;
	const ECaseRedResult R = FCaseSim::Red(*Case, State, A, B, &Stamped);
	if (R != ECaseRedResult::Invalid)
	{
		OnRedJudged.Broadcast(R, Stamped);
		AfterAction({});
	}
	return R;
}

bool UInvestigationSubsystem::Pin(ECaseSlot Slot, FName Value)
{
	if (!IsReady() || !FCaseSim::Pin(*Case, State, Slot, Value))
	{
		return false;
	}
	OnCaseChanged.Broadcast();
	return true;
}

void UInvestigationSubsystem::Unpin(ECaseSlot Slot)
{
	if (IsReady())
	{
		FCaseSim::Unpin(State, Slot);
		OnCaseChanged.Broadcast();
	}
}

void UInvestigationSubsystem::EndNight()
{
	if (IsReady())
	{
		DoEndNight();
	}
}

bool UInvestigationSubsystem::Report(FName Attached)
{
	if (!Case || !FCaseSim::CanReport(State))
	{
		return false;
	}
	FCaseSim::Close(*Case, State, Attached);
	UE_LOG(LogInvestigation, Display, TEXT("[Investigation] report sent: ending %s, rating %s"), *State.Ending.ToString(), *State.Grade.ToString());
	OnCaseChanged.Broadcast();
	OnCaseClosed.Broadcast(State.Ending, State.Grade);
	return true;
}

void UInvestigationSubsystem::AfterAction(const TArray<FName>& Gained)
{
	if (!Gained.IsEmpty())
	{
		OnCardsGained.Broadcast(Gained);
	}
	OnCaseChanged.Broadcast();
	if (!State.bClosed && FCaseSim::NightOver(*Case, State))
	{
		DoEndNight();
	}
}

void UInvestigationSubsystem::DoEndNight()
{
	TArray<FName> Gained;
	const FName Line = FCaseSim::EndNight(*Case, State, &Gained);
	OnCaseChanged.Broadcast();
	if (!Gained.IsEmpty())
	{
		OnCardsGained.Broadcast(Gained);
	}
	if (State.bClosed)
	{
		UE_LOG(LogInvestigation, Display, TEXT("[Investigation] the ship sailed: ending %s, rating %s"), *State.Ending.ToString(), *State.Grade.ToString());
		OnCaseClosed.Broadcast(State.Ending, State.Grade);
		return;
	}
	UE_LOG(LogInvestigation, Display, TEXT("[Investigation] dawn: %s; night %d starts %s"), *Line.ToString(), State.Night + 1, *GetClockText());
	OnDawn.Broadcast(Line, State.Night);
	if (!Case->Hub.IsNone())
	{
		OnDrive.Broadcast(State.bAtHub); // dawn is in the darkroom: the player wakes at the office
	}
}

void UInvestigationSubsystem::RegisterSubject(UPhotoSubjectComponent* Subject)
{
	Subjects.AddUnique(Subject);
}

void UInvestigationSubsystem::UnregisterSubject(UPhotoSubjectComponent* Subject)
{
	Subjects.Remove(Subject);
}
