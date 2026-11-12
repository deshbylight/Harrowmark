// The press camera (GDD 9.1). Attach it to the view camera. While raised it rates every subject in front of it
// (frame, size, occlusion traces, flash range or scene light) for the viewfinder brackets; Shoot() fires the
// flash, captures the frame as a black-and-white print and hands the hits to the case.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CaseTypes.h"
#include "PhotoCameraComponent.generated.h"

class UPhotoSubjectComponent;
class USceneCaptureComponent2D;
class USpotLightComponent;
class UTextureRenderTarget2D;

/** How one subject looks through the viewfinder right now. */
USTRUCT(BlueprintType)
struct INVESTIGATION_API FPhotoSubjectView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Photo") TWeakObjectPtr<UPhotoSubjectComponent> Subject;
	UPROPERTY(BlueprintReadOnly, Category = "Photo") FName ShotId;
	/** Screen rectangle in frame units (0..1, origin top left), clamped to the frame. */
	UPROPERTY(BlueprintReadOnly, Category = "Photo") FVector2D Min = FVector2D::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Photo") FVector2D Max = FVector2D::ZeroVector;
	/** Projected size as a fraction of the frame height. */
	UPROPERTY(BlueprintReadOnly, Category = "Photo") float Size = 0.f;
	/** Fraction of sample points with a clear line from the lens. */
	UPROPERTY(BlueprintReadOnly, Category = "Photo") float Visible = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Photo") bool bValid = false;
	UPROPERTY(BlueprintReadOnly, Category = "Photo") bool bSharp = false;
	/** What the case says about shooting it now (Ok, AlreadyDone, Tutorial, NoFilm...). */
	UPROPERTY(BlueprintReadOnly, Category = "Photo") ECaseActionResult CaseResult = ECaseActionResult::Invalid;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShutter, ECaseActionResult, Result, UTexture*, Print);

UCLASS(ClassGroup = (Investigation), meta = (BlueprintSpawnableComponent))
class INVESTIGATION_API UPhotoCameraComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPhotoCameraComponent();

	// ---- tunables (GDD 9.1) ----
	/** Viewfinder vertical field of view, degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Photo") float VerticalFOV = 34.f;
	/** Print aspect (width / height); the viewfinder frame uses the same. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Photo") float FrameAspect = 4.f / 3.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Photo") float ValidSize = 0.15f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Photo") float SharpSize = 0.35f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Photo") float MinVisible = 0.6f;
	/** Flash reach, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Photo") float FlashRange = 600.f;
	/** Subjects further than this are not even rated, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Photo") float MaxRange = 2500.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Photo") float FlashIntensity = 400000.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Photo") float FlashSeconds = 0.08f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Photo") FIntPoint PrintSize = FIntPoint(960, 720);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Photo") float GrainyFilmGrain = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Photo") float SharpFilmGrain = 0.25f;

	/** Horizontal FOV, degrees, for a viewport of this aspect (the view camera uses it while raised). */
	UFUNCTION(BlueprintPure, Category = "Photo") float HorizontalFOV(float Aspect) const;

	/** Rates every registered subject in front of the lens. Sorted: valid first, then by size. */
	UFUNCTION(BlueprintCallable, Category = "Photo") TArray<FPhotoSubjectView> Evaluate() const;

	/** Rates one subject (also used by tests). */
	FPhotoSubjectView Rate(const UPhotoSubjectComponent& Subject) const;

	/** Fires if at least one valid subject the case accepts is in frame; otherwise refuses and returns why. */
	UFUNCTION(BlueprintCallable, Category = "Photo") ECaseActionResult Shoot();

	UPROPERTY(BlueprintAssignable, Category = "Photo") FOnShutter OnShutter;

	USpotLightComponent* GetFlash() const { return Flash; }

protected:
	virtual void OnRegister() override;

private:
	bool Project(const FVector& World, FVector2D& OutNdc) const;
	UTextureRenderTarget2D* Capture(bool bSharp);
	void FlashOff();

	UPROPERTY() TObjectPtr<USceneCaptureComponent2D> CaptureComponent;
	UPROPERTY() TObjectPtr<USpotLightComponent> Flash;
	FTimerHandle FlashTimer;
};
