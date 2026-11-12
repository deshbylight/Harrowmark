#include "HarrowGameMode.h"

#include "Camera/PlayerCameraManager.h"
#include "CaseDefinition.h"
#include "CaseRules.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "HarrowCharacter.h"
#include "InvestigationSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

const FName AHarrowGameMode::StartTag(TEXT("Start"));
const FName AHarrowGameMode::CarTag(TEXT("Car"));
const FName AHarrowGameMode::OfficeTag(TEXT("Office"));
const FName AHarrowGameMode::DawnTag(TEXT("Dawn"));

AHarrowGameMode::AHarrowGameMode()
{
	DefaultPawnClass = AHarrowCharacter::StaticClass();
	HarbourMap = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Maps/L_Pier5.L_Pier5")));
	HubMap = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Maps/L_Office.L_Office")));
}

bool AHarrowGameMode::IsHubMap() const
{
	const UWorld* World = GetWorld();
	return World && UWorld::RemovePIEPrefix(World->GetPackage()->GetName()) == HubMap.ToSoftObjectPath().GetLongPackageName();
}

void AHarrowGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	const FString Arrive = UGameplayStatics::ParseOption(Options, TEXT("Arrive"));
	Arrival = Arrive.IsEmpty() ? NAME_None : FName(*Arrive);
}

void AHarrowGameMode::StartPlay()
{
	// The case starts before actors begin play, so subjects and gates see the right night when they register.
	UInvestigationSubsystem* Inv = UInvestigationSubsystem::Get(this);
	if (Inv && !Inv->GetCase())
	{
		ensureMsgf(Inv->StartCase(), TEXT("No case could be loaded (UInvestigationSettings Case / FallbackJson)"));
	}
	Super::StartPlay();
	if (!Inv || !Inv->GetCase())
	{
		return;
	}
	Inv->OnDrive.AddDynamic(this, &AHarrowGameMode::HandleDrive);
	Inv->OnDawn.AddDynamic(this, &AHarrowGameMode::HandleDawn);
	if (!Inv->GetCase()->Hub.IsNone() && Inv->GetState().bAtHub != IsHubMap())
	{
		// Play started on the other map (an editor session): go where the case says the detective is.
		bRedirect = true;
		Travel();
		return;
	}
	if (!Arrival.IsNone())
	{
		Fade(1.f, 0.f);
	}
}

void AHarrowGameMode::EndPlay(const EEndPlayReason::Type Reason)
{
	if (UInvestigationSubsystem* Inv = UInvestigationSubsystem::Get(this))
	{
		Inv->OnDrive.RemoveDynamic(this, &AHarrowGameMode::HandleDrive);
		Inv->OnDawn.RemoveDynamic(this, &AHarrowGameMode::HandleDawn);
	}
	Super::EndPlay(Reason);
}

AActor* AHarrowGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (AActor* Start = FindStart(Arrival.IsNone() ? StartTag : Arrival))
	{
		return Start;
	}
	return Super::ChoosePlayerStart_Implementation(Player);
}

AActor* AHarrowGameMode::FindStart(FName Tag) const
{
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		if (It->PlayerStartTag == Tag)
		{
			return *It;
		}
	}
	return nullptr;
}

void AHarrowGameMode::HandleDawn(FName /*MonologueLine*/, int32 /*NewNight*/)
{
	bDawnPending = true;
}

void AHarrowGameMode::HandleDrive(bool /*bAtHub*/)
{
	// Dawn fires this after a drive that ran out the night: one travel, to wherever the state ends up.
	FTimerManager& Timers = GetWorldTimerManager();
	if (Timers.IsTimerActive(TravelTimer))
	{
		return;
	}
	Fade(0.f, 1.f);
	Timers.SetTimer(TravelTimer, this, &AHarrowGameMode::Travel, FMath::Max(FadeSeconds, 0.01f), false);
}

void AHarrowGameMode::Travel()
{
	const UInvestigationSubsystem* Inv = UInvestigationSubsystem::Get(this);
	if (!Inv || !Inv->GetCase())
	{
		return;
	}
	const bool bAtHub = Inv->GetState().bAtHub;
	// A redirect into a case nobody has touched yet is the case start, not an arrival by car.
	const bool bFresh = bRedirect && Inv->GetState().Taken.IsEmpty() && Inv->GetState().Hand.IsEmpty();
	const FName Tag = bAtHub ? (bDawnPending ? DawnTag : OfficeTag) : (bFresh ? StartTag : CarTag);
	bDawnPending = false;
	bRedirect = false;
	if (bAtHub == IsHubMap())
	{
		// Dawn while already at the office: no map change, cut to the darkroom.
		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		const AActor* Start = FindStart(Tag);
		if (PC && PC->GetPawn() && Start)
		{
			PC->GetPawn()->TeleportTo(Start->GetActorLocation(), Start->GetActorRotation());
			PC->SetControlRotation(Start->GetActorRotation());
		}
		Fade(1.f, 0.f);
		return;
	}
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, bAtHub ? HubMap : HarbourMap, true,
		FString::Printf(TEXT("Arrive=%s"), *Tag.ToString()));
}

void AHarrowGameMode::Fade(float From, float To) const
{
	const APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC && PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StartCameraFade(From, To, FadeSeconds, FLinearColor::Black, false, true);
	}
}
