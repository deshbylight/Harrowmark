// Placeable case actors for the map (spawned by Levels/pier5_greybox.py through `jf level`):
//   ACaseSubjectActor - a thing worth a photograph: a block mesh sized by MeshSize plus a photo subject;
//   ACasePersonActor  - a person: mannequin, talk, an optional photo subject (the timed tableaux) and
//                       presence by night and by shot (Varga leaves CL-2 while he is fishing);
//   ACaseGateActor    - an obstacle that disappears when its gate opens (cabin door, Varga's locker);
//   ACaseDriveActor   - the detective's car at the harbour and the office door: E is the drive.
// Component settings are set per instance in the spec with "props": {"subject.shot_id": "SH_Wound", ...}.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CaseActors.generated.h"

class UCapsuleComponent;
class UCaseGateComponent;
class UDriveComponent;
class UPhotoSubjectComponent;
class USkeletalMeshComponent;
class UStaticMeshComponent;
class UTalkComponent;

UCLASS()
class HARROWMARK_API ACaseSubjectActor : public AActor
{
	GENERATED_BODY()

public:
	ACaseSubjectActor();

	/** Size of the block mesh in cm (the default cube is 100 cm). Zero: no mesh, the subject is part of the set. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Case") FVector MeshSize = FVector(30.f);

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TObjectPtr<UStaticMeshComponent> Mesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TObjectPtr<UPhotoSubjectComponent> Subject;
};

UCLASS()
class HARROWMARK_API ACasePersonActor : public AActor
{
	GENERATED_BODY()

public:
	ACasePersonActor();

	/** Standing height in cm (case data "cm"); the mannequin is scaled to it. 0 keeps the mannequin's own. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Case") float HeightCm = 0.f;
	/** 1-based nights this person stands here. Empty: every night. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Case") TArray<int32> Nights;
	/** Here only while this shot is present (the second figure of a timed tableau). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Case") FName ShownWithShot;
	/** Away while any of these shots is present (he is in a tableau, not at his usual spot). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Case") TArray<FName> HiddenWithShots;

	UFUNCTION(BlueprintPure, Category = "Case") bool IsHere() const { return bHere; }

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TObjectPtr<UCapsuleComponent> Capsule;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TObjectPtr<USkeletalMeshComponent> Mesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TObjectPtr<UTalkComponent> Talk;
	/** Inactive unless its shot id is set; this actor, not the subject, decides when the person is here. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TObjectPtr<UPhotoSubjectComponent> Subject;

private:
	UFUNCTION() void Refresh();
	bool bHere = true;
};

UCLASS()
class HARROWMARK_API ACaseGateActor : public AActor
{
	GENERATED_BODY()

public:
	ACaseGateActor();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Case") FVector MeshSize = FVector(120.f, 10.f, 220.f);

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TObjectPtr<UStaticMeshComponent> Mesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TObjectPtr<UCaseGateComponent> Gate;
};

UCLASS()
class HARROWMARK_API ACaseDriveActor : public AActor
{
	GENERATED_BODY()

public:
	ACaseDriveActor();

	/** Size of the block mesh in cm (a sedan by default). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Case") FVector MeshSize = FVector(480.f, 180.f, 145.f);

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TObjectPtr<UStaticMeshComponent> Mesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Case") TObjectPtr<UDriveComponent> Drive;
};
