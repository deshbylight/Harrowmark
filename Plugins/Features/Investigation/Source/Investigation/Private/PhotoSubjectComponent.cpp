#include "PhotoSubjectComponent.h"

#include "CaseDefinition.h"
#include "GameFramework/Actor.h"
#include "InvestigationSubsystem.h"

UPhotoSubjectComponent::UPhotoSubjectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPhotoSubjectComponent::GetSamplePoints(TArray<FVector>& Out) const
{
	const FTransform& T = GetComponentTransform();
	Out.Add(T.GetLocation());
	for (int32 I = 0; I < 8; ++I)
	{
		const FVector Corner((I & 1) ? BoxExtent.X : -BoxExtent.X, (I & 2) ? BoxExtent.Y : -BoxExtent.Y, (I & 4) ? BoxExtent.Z : -BoxExtent.Z);
		Out.Add(T.TransformPosition(Corner));
	}
}

void UPhotoSubjectComponent::BeginPlay()
{
	Super::BeginPlay();
	if (ShotId.IsNone())
	{
		return; // inactive: actors that only sometimes carry a shot (people) leave it empty
	}
	if (UInvestigationSubsystem* Inv = UInvestigationSubsystem::Get(this))
	{
		Inv->RegisterSubject(this);
		Inv->OnCaseChanged.AddDynamic(this, &UPhotoSubjectComponent::Refresh);
		Refresh();
	}
}

void UPhotoSubjectComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	if (UInvestigationSubsystem* Inv = UInvestigationSubsystem::Get(this))
	{
		Inv->UnregisterSubject(this);
		Inv->OnCaseChanged.RemoveDynamic(this, &UPhotoSubjectComponent::Refresh);
	}
	Super::EndPlay(Reason);
}

void UPhotoSubjectComponent::Refresh()
{
	const UInvestigationSubsystem* Inv = UInvestigationSubsystem::Get(this);
	const UCaseDefinition* Case = Inv ? Inv->GetCase() : nullptr;
	if (!Case)
	{
		return;
	}
	ensureMsgf(Case->FindShot(ShotId), TEXT("%s: unknown shot %s"), *GetPathName(), *ShotId.ToString());
	bPresent = FCaseSim::ShotPresent(*Case, Inv->GetState(), ShotId);
	if (bHideOwnerWhenGone && GetOwner())
	{
		GetOwner()->SetActorHiddenInGame(!bPresent);
		GetOwner()->SetActorEnableCollision(bPresent);
	}
}
