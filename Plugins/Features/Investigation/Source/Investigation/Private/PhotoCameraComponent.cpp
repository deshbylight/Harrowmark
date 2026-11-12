#include "PhotoCameraComponent.h"

#include "CaseDefinition.h"
#include "CaseRules.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SpotLightComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "InvestigationSubsystem.h"
#include "PhotoSubjectComponent.h"
#include "TimerManager.h"
#include "UObject/Package.h"

UPhotoCameraComponent::UPhotoCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPhotoCameraComponent::OnRegister()
{
	Super::OnRegister();
	const UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld() || !GetOwner())
	{
		return;
	}
	if (!CaptureComponent)
	{
		CaptureComponent = NewObject<USceneCaptureComponent2D>(GetOwner(), TEXT("PrintCapture"), RF_Transient);
		CaptureComponent->SetupAttachment(this);
		CaptureComponent->bCaptureEveryFrame = false;
		CaptureComponent->bCaptureOnMovement = false;
		CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
		CaptureComponent->FOVAngle = HorizontalFOV(FrameAspect);
		CaptureComponent->HiddenActors.Add(GetOwner());
		FPostProcessSettings& PP = CaptureComponent->PostProcessSettings;
		PP.bOverride_ColorSaturation = true;
		PP.ColorSaturation = FVector4(0.f, 0.f, 0.f, 1.f);
		PP.bOverride_ColorContrast = true;
		PP.ColorContrast = FVector4(1.15f, 1.15f, 1.15f, 1.f);
		PP.bOverride_VignetteIntensity = true;
		PP.VignetteIntensity = 0.6f;
		PP.bOverride_FilmGrainIntensity = true;
		CaptureComponent->PostProcessBlendWeight = 1.f;
		CaptureComponent->RegisterComponent();
	}
	if (!Flash)
	{
		Flash = NewObject<USpotLightComponent>(GetOwner(), TEXT("Flash"), RF_Transient);
		Flash->SetupAttachment(this);
		Flash->SetIntensityUnits(ELightUnits::Candelas);
		Flash->SetIntensity(FlashIntensity);
		Flash->SetAttenuationRadius(FlashRange * 1.6f);
		Flash->SetInnerConeAngle(20.f);
		Flash->SetOuterConeAngle(45.f);
		Flash->SetLightColor(FLinearColor(1.f, 0.97f, 0.92f));
		Flash->SetCastShadows(true);
		Flash->SetVisibility(false);
		Flash->RegisterComponent();
	}
}

float UPhotoCameraComponent::HorizontalFOV(float Aspect) const
{
	const float HalfV = FMath::DegreesToRadians(VerticalFOV * 0.5f);
	return FMath::RadiansToDegrees(2.f * FMath::Atan(FMath::Tan(HalfV) * Aspect));
}

bool UPhotoCameraComponent::Project(const FVector& World, FVector2D& OutNdc) const
{
	const FVector Local = GetComponentTransform().InverseTransformPositionNoScale(World);
	if (Local.X < 5.f)
	{
		return false;
	}
	const float TanV = FMath::Tan(FMath::DegreesToRadians(VerticalFOV * 0.5f));
	const float TanH = TanV * FrameAspect;
	OutNdc = FVector2D(Local.Y / (Local.X * TanH), Local.Z / (Local.X * TanV));
	return true;
}

FPhotoSubjectView UPhotoCameraComponent::Rate(const UPhotoSubjectComponent& Subject) const
{
	FPhotoSubjectView View;
	View.Subject = const_cast<UPhotoSubjectComponent*>(&Subject);
	View.ShotId = Subject.ShotId;

	TArray<FVector> Points;
	Subject.GetSamplePoints(Points);
	FVector2D Center;
	if (!Project(Points[0], Center) || FMath::Abs(Center.X) > 1.f || FMath::Abs(Center.Y) > 1.f)
	{
		return View;
	}
	FVector2D Lo(1.f, 1.f), Hi(-1.f, -1.f);
	for (const FVector& P : Points)
	{
		FVector2D Ndc;
		if (Project(P, Ndc))
		{
			Ndc = FVector2D(FMath::Clamp(Ndc.X, -1.f, 1.f), FMath::Clamp(Ndc.Y, -1.f, 1.f));
			Lo = FVector2D::Min(Lo, Ndc);
			Hi = FVector2D::Max(Hi, Ndc);
		}
	}
	// Size in frame heights: the taller of the projected height and the projected width (a body lying flat counts).
	View.Size = FMath::Max((Hi.Y - Lo.Y) * 0.5f, (Hi.X - Lo.X) * 0.5f * FrameAspect);
	View.Min = FVector2D((Lo.X + 1.f) * 0.5f, (1.f - Hi.Y) * 0.5f);
	View.Max = FVector2D((Hi.X + 1.f) * 0.5f, (1.f - Lo.Y) * 0.5f);

	const UWorld* World = GetWorld();
	const FVector Lens = GetComponentLocation();
	FCollisionQueryParams Params(SCENE_QUERY_STAT(PhotoSubject), false);
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(Subject.GetOwner());
	// A hit inside the subject's own box is the subject itself (a gouge in a post, a log on a shelf), not a blocker.
	const FTransform& Box = Subject.GetComponentTransform();
	const FVector Slack = Subject.BoxExtent + FVector(5.f);
	auto InBox = [&](const FVector& W)
	{
		const FVector L = Box.InverseTransformPosition(W).GetAbs();
		return L.X <= Slack.X && L.Y <= Slack.Y && L.Z <= Slack.Z;
	};
	int32 Clear = 0;
	for (const FVector& P : Points)
	{
		FHitResult Hit;
		if (!World || !World->LineTraceSingleByChannel(Hit, Lens, P, ECC_Visibility, Params) || InBox(Hit.ImpactPoint))
		{
			++Clear;
		}
	}
	View.Visible = static_cast<float>(Clear) / Points.Num();

	const bool bLit = Subject.bLitByScene || FVector::Dist(Lens, Points[0]) <= FlashRange;
	View.bValid = Subject.IsPresent() && bLit && View.Size >= ValidSize && View.Visible >= MinVisible;
	View.bSharp = View.bValid && View.Size >= SharpSize;

	const UInvestigationSubsystem* Inv = UInvestigationSubsystem::Get(this);
	if (Inv && Inv->GetCase())
	{
		View.CaseResult = FCaseSim::CanShoot(*Inv->GetCase(), Inv->GetState(), Subject.ShotId);
	}
	return View;
}

TArray<FPhotoSubjectView> UPhotoCameraComponent::Evaluate() const
{
	TArray<FPhotoSubjectView> Out;
	const UInvestigationSubsystem* Inv = UInvestigationSubsystem::Get(this);
	if (!Inv)
	{
		return Out;
	}
	const FVector Lens = GetComponentLocation();
	for (const TWeakObjectPtr<UPhotoSubjectComponent>& Weak : Inv->GetSubjects())
	{
		const UPhotoSubjectComponent* Subject = Weak.Get();
		if (Subject && Subject->IsPresent() && FVector::DistSquared(Lens, Subject->GetComponentLocation()) <= FMath::Square(MaxRange))
		{
			FPhotoSubjectView View = Rate(*Subject);
			if (View.Size > 0.f)
			{
				Out.Add(View);
			}
		}
	}
	Out.Sort([](const FPhotoSubjectView& A, const FPhotoSubjectView& B) { return A.bValid != B.bValid ? A.bValid : A.Size > B.Size; });
	return Out;
}

ECaseActionResult UPhotoCameraComponent::Shoot()
{
	UInvestigationSubsystem* Inv = UInvestigationSubsystem::Get(this);
	if (!Inv || !Inv->GetCase())
	{
		return ECaseActionResult::Invalid;
	}
	TArray<FCaseShotHit> Hits;
	ECaseActionResult Refusal = ECaseActionResult::Invalid;
	bool bAnySharp = false;
	for (const FPhotoSubjectView& View : Evaluate())
	{
		if (!View.bValid)
		{
			continue;
		}
		if (View.CaseResult == ECaseActionResult::Ok)
		{
			Hits.Emplace(View.ShotId, View.bSharp);
			bAnySharp |= View.bSharp || Inv->GetCase()->FindShot(View.ShotId)->bForced;
		}
		else if (Refusal == ECaseActionResult::Invalid)
		{
			Refusal = View.CaseResult;
		}
	}
	if (Hits.IsEmpty())
	{
		OnShutter.Broadcast(Refusal, nullptr);
		return Refusal;
	}
	UTextureRenderTarget2D* Print = Capture(bAnySharp);
	const ECaseActionResult R = Inv->Shoot(Hits, Print);
	OnShutter.Broadcast(R, Print);
	return R;
}

UTextureRenderTarget2D* UPhotoCameraComponent::Capture(bool bSharp)
{
	if (!CaptureComponent)
	{
		return nullptr;
	}
	// Outer is the transient package, not this component: prints live in the game instance's subsystem and
	// survive the drive between maps; an outer in the old world would keep that world alive.
	UTextureRenderTarget2D* Target = NewObject<UTextureRenderTarget2D>(GetTransientPackage());
	Target->RenderTargetFormat = RTF_RGBA8_SRGB;
	Target->ClearColor = FLinearColor::Black;
	Target->InitAutoFormat(PrintSize.X, PrintSize.Y);
	Target->UpdateResourceImmediate(true);

	CaptureComponent->PostProcessSettings.FilmGrainIntensity = bSharp ? SharpFilmGrain : GrainyFilmGrain;
	CaptureComponent->TextureTarget = Target;
	if (Flash)
	{
		Flash->SetVisibility(true);
		if (const UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(FlashTimer, this, &UPhotoCameraComponent::FlashOff, FlashSeconds, false);
		}
	}
	// Deferred: rendered with the next frame, so the flash that was just switched on is in the print.
	CaptureComponent->CaptureSceneDeferred();
	return Target;
}

void UPhotoCameraComponent::FlashOff()
{
	if (Flash)
	{
		Flash->SetVisibility(false);
	}
}
