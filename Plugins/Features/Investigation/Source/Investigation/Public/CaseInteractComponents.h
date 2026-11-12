// Things the detective uses with E: people (talk), documents and objects (inspect, always free), the car
// (the drive between the harbour and the office) and gates that open when a lie is stamped (CASE01 4.6). The character traces for UCaseInteractComponent; the UI
// listens to UInvestigationSubsystem::OnTalkRequested / OnInspectRequested.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CaseInteractComponents.generated.h"

UCLASS(Abstract, ClassGroup = (Investigation))
class INVESTIGATION_API UCaseInteractComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	/** Short prompt next to the E key ("Talk", "Read"). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact") FText Prompt;
	/** Reach from the detective's view, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact") float Range = 220.f;

	virtual bool CanInteract() const { return true; }
	virtual void Interact() PURE_VIRTUAL(UCaseInteractComponent::Interact, );
};

/** On a person: opens the topic list for Speaker (a people id in the case). */
UCLASS(ClassGroup = (Investigation), meta = (BlueprintSpawnableComponent))
class INVESTIGATION_API UTalkComponent : public UCaseInteractComponent
{
	GENERATED_BODY()

public:
	UTalkComponent();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talk") FName Speaker;
	virtual bool CanInteract() const override;
	virtual void Interact() override;
};

/** On a readable or inspectable thing: shows its text. Free, never costs time (GDD 9.4). */
UCLASS(ClassGroup = (Investigation), meta = (BlueprintSpawnableComponent))
class INVESTIGATION_API UInspectComponent : public UCaseInteractComponent
{
	GENERATED_BODY()

public:
	UInspectComponent();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inspect") FText Title;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inspect", meta = (MultiLine = true)) FText Body;
	virtual void Interact() override;
};

/** On the detective's car at the harbour and on the office door: the drive (10 min of case time each way,
 *  GDD 9.4). The game mode listens to UInvestigationSubsystem::OnDrive and moves the player between maps. */
UCLASS(ClassGroup = (Investigation), meta = (BlueprintSpawnableComponent))
class INVESTIGATION_API UDriveComponent : public UCaseInteractComponent
{
	GENERATED_BODY()

public:
	UDriveComponent();
	virtual bool CanInteract() const override;
	virtual void Interact() override;
};

/** On a door, a barrier or a closed cabin: blocks until its gate opens, then hides the owner (or shows it with bShowWhenOpen). */
UCLASS(ClassGroup = (Investigation), meta = (BlueprintSpawnableComponent))
class INVESTIGATION_API UCaseGateComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gate") FName Gate;
	/** False: the owner is the obstacle and disappears. True: the owner is what lies behind and appears. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gate") bool bShowWhenOpen = false;

	UFUNCTION(BlueprintPure, Category = "Gate") bool IsOpen() const { return bOpen; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
	UFUNCTION() void Refresh();
	bool bOpen = false;
};
