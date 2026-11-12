#include "CaseInteractComponents.h"

#include "CaseDefinition.h"
#include "GameFramework/Actor.h"
#include "InvestigationSubsystem.h"

UTalkComponent::UTalkComponent()
{
	Prompt = NSLOCTEXT("Investigation", "Talk", "Talk");
}

bool UTalkComponent::CanInteract() const
{
	const UInvestigationSubsystem* Inv = UInvestigationSubsystem::Get(this);
	return Inv && Inv->GetCase() && !Inv->GetState().bClosed;
}

void UTalkComponent::Interact()
{
	if (UInvestigationSubsystem* Inv = UInvestigationSubsystem::Get(this))
	{
		Inv->OnTalkRequested.Broadcast(this);
	}
}

UInspectComponent::UInspectComponent()
{
	Prompt = NSLOCTEXT("Investigation", "Read", "Read");
}

void UInspectComponent::Interact()
{
	if (UInvestigationSubsystem* Inv = UInvestigationSubsystem::Get(this))
	{
		Inv->OnInspectRequested.Broadcast(this);
	}
}

UDriveComponent::UDriveComponent()
{
	Prompt = NSLOCTEXT("Investigation", "Drive", "Drive");
	Range = 300.f;
}

bool UDriveComponent::CanInteract() const
{
	const UInvestigationSubsystem* Inv = UInvestigationSubsystem::Get(this);
	return Inv && Inv->GetCase() && !Inv->GetCase()->Hub.IsNone() && !Inv->GetState().bClosed;
}

void UDriveComponent::Interact()
{
	if (UInvestigationSubsystem* Inv = UInvestigationSubsystem::Get(this))
	{
		Inv->Drive();
	}
}

void UCaseGateComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UInvestigationSubsystem* Inv = UInvestigationSubsystem::Get(this))
	{
		Inv->OnCaseChanged.AddDynamic(this, &UCaseGateComponent::Refresh);
	}
	Refresh();
}

void UCaseGateComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	if (UInvestigationSubsystem* Inv = UInvestigationSubsystem::Get(this))
	{
		Inv->OnCaseChanged.RemoveDynamic(this, &UCaseGateComponent::Refresh);
	}
	Super::EndPlay(Reason);
}

void UCaseGateComponent::Refresh()
{
	const UInvestigationSubsystem* Inv = UInvestigationSubsystem::Get(this);
	bOpen = Inv && Inv->GetState().Gates.Contains(Gate);
	if (AActor* Owner = GetOwner())
	{
		const bool bShown = bShowWhenOpen ? bOpen : !bOpen;
		Owner->SetActorHiddenInGame(!bShown);
		Owner->SetActorEnableCollision(bShown);
	}
}
