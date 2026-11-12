// Marks something worth a photograph (CASE01 4.1). Place it on the subject's actor, size the box to what the
// frame must hold; UPhotoCameraComponent projects the box into the viewfinder and decides valid / sharp.
// The owner hides itself while its shot is not present (expired after its night, outside its timed window).

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PhotoSubjectComponent.generated.h"

UCLASS(ClassGroup = (Investigation), meta = (BlueprintSpawnableComponent))
class INVESTIGATION_API UPhotoSubjectComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPhotoSubjectComponent();

	/** Shot id in the case data (SH_...). None: inactive (not registered, never hides the owner). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Subject") FName ShotId;
	/** Half size of the box that must be in frame, in component space. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Subject") FVector BoxExtent = FVector(30.f);
	/** True when a scene light is enough for a sharp shot without the flash (lamp pools, lit windows). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Subject") bool bLitByScene = false;
	/** Hide the owning actor while the shot is not present. Off for subjects that are part of the set. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Subject") bool bHideOwnerWhenGone = true;

	/** World-space points the camera tests for framing and occlusion: centre and the 8 box corners. */
	void GetSamplePoints(TArray<FVector>& Out) const;
	bool IsPresent() const { return bPresent; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
	UFUNCTION() void Refresh();
	bool bPresent = true;
};
