// The detective (GDD 8): first person with a full body. Hold RMB to raise the camera to the eye (the view
// narrows to the lens, no view switch), LMB shoots, E uses what is in front, F the flashlight, Tab the board,
// C crouch, Shift jog. Input actions and the mapping context are built in C++ so the controls live in one place.
//
// Two copies of the body, one per audience:
//   Mesh (ACharacter's)  - hidden from the player's own view; casts the shadow (bCastHiddenShadow) and is what
//                          ray-traced reflections show (the back-bar mirror, A27);
//   ViewBody             - only the player sees it: looking down, the head bone hidden, no shadow, not ray traced.
// Each runs its own anim instance: a leader-pose follower cannot hide a bone of its own.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HarrowCharacter.generated.h"

class UCameraComponent;
class UCaseInteractComponent;
class UInputAction;
class UInputMappingContext;
class UPhotoCameraComponent;
class USpotLightComponent;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnViewfinder, bool, bRaised);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBoardRequested);

UCLASS()
class HARROWMARK_API AHarrowCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AHarrowCharacter();

	// ---- tunables (docs/design/METRICS) ----
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detective") float WalkSpeed = 300.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detective") float JogSpeed = 500.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detective") float ViewfinderSpeed = 120.f;
	/** Horizontal FOV, degrees, while walking; raising the camera narrows it to the lens (UPhotoCameraComponent). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detective") float WalkFOV = 80.f;
	/** Seconds to raise the camera to the eye or lower it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detective") float RaiseSeconds = 0.25f;
	/** Eye in front of the capsule centre, cm: clears the neck when looking down. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detective") float EyeForward = 16.f;
	/** Extra forward eye offset, cm, when looking straight down (scaled by the pitch): keeps the chest out of the view. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detective") float LookDownForward = 22.f;
	/** Lowest view pitch, degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detective") float MinPitch = -78.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detective") float MouseSensitivity = 0.6f;
	/** Viewfinder look speed, scaled down because the lens is long. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detective") float ViewfinderSensitivity = 0.3f;

	UFUNCTION(BlueprintPure, Category = "Detective") bool IsViewfinderRaised() const { return bRaised; }
	UFUNCTION(BlueprintPure, Category = "Detective") UPhotoCameraComponent* GetPhotoCamera() const { return PhotoCamera; }
	/** What E would use right now (null when nothing is in reach). */
	UFUNCTION(BlueprintPure, Category = "Detective") UCaseInteractComponent* GetFocus() const { return Focus; }

	/** Blocks movement, look and the camera while a screen (talk, board, darkroom) is open. */
	UFUNCTION(BlueprintCallable, Category = "Detective") void SetInputBlocked(bool bBlocked);
	/** Raises the camera to the eye or lowers it, as holding RMB does. */
	UFUNCTION(BlueprintCallable, Category = "Detective") void SetRaised(bool bInRaised) { bInRaised ? RaiseCamera() : LowerCamera(); }

	UPROPERTY(BlueprintAssignable, Category = "Detective") FOnViewfinder OnViewfinder;
	UPROPERTY(BlueprintAssignable, Category = "Detective") FOnBoardRequested OnBoardRequested;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
	virtual void NotifyControllerChanged() override;

private:
	void BuildInput();
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void RaiseCamera();
	void LowerCamera();
	void Shoot();
	void Use();
	void ToggleFlashlight();
	void ToggleCrouch();
	void StartJog();
	void StopJog();
	void Board();
	void UpdateFocus();
	void UpdateSpeed();
	void UpdateView(float DeltaSeconds);
	float ViewfinderFOV() const;

	UPROPERTY(VisibleAnywhere, Category = "Detective") TObjectPtr<UCameraComponent> EyeCamera;
	UPROPERTY(VisibleAnywhere, Category = "Detective") TObjectPtr<USkeletalMeshComponent> ViewBody;
	UPROPERTY(VisibleAnywhere, Category = "Detective") TObjectPtr<UPhotoCameraComponent> PhotoCamera;
	UPROPERTY(VisibleAnywhere, Category = "Detective") TObjectPtr<USpotLightComponent> Flashlight;

	UPROPERTY(Transient) TObjectPtr<UInputMappingContext> Mapping;
	UPROPERTY(Transient) TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(Transient) TObjectPtr<UInputAction> LookAction;
	UPROPERTY(Transient) TObjectPtr<UInputAction> RaiseAction;
	UPROPERTY(Transient) TObjectPtr<UInputAction> ShootAction;
	UPROPERTY(Transient) TObjectPtr<UInputAction> UseAction;
	UPROPERTY(Transient) TObjectPtr<UInputAction> FlashlightAction;
	UPROPERTY(Transient) TObjectPtr<UInputAction> CrouchAction;
	UPROPERTY(Transient) TObjectPtr<UInputAction> JogAction;
	UPROPERTY(Transient) TObjectPtr<UInputAction> BoardAction;

	UPROPERTY(Transient) TObjectPtr<UCaseInteractComponent> Focus;

	bool bRaised = false;
	/** 0 = lowered, 1 = at the eye; eases the FOV between WalkFOV and the lens. */
	float RaiseAlpha = 0.f;
	bool bJogging = false;
	bool bBlocked = false;
};
