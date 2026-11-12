// The detective (GDD 8): third person over the shoulder; hold RMB for the first-person viewfinder, LMB shoots,
// E uses what is in front, F the flashlight, Tab the board, C crouch, Shift jog. Input actions and the mapping
// context are built in C++ so the controls live in one place.

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
class USpringArmComponent;
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detective") float ShoulderFOV = 75.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detective") float MouseSensitivity = 0.6f;
	/** Viewfinder look speed, scaled down because the lens is long. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detective") float ViewfinderSensitivity = 0.3f;

	UFUNCTION(BlueprintPure, Category = "Detective") bool IsViewfinderRaised() const { return bRaised; }
	UFUNCTION(BlueprintPure, Category = "Detective") UPhotoCameraComponent* GetPhotoCamera() const { return PhotoCamera; }
	/** What E would use right now (null when nothing is in reach). */
	UFUNCTION(BlueprintPure, Category = "Detective") UCaseInteractComponent* GetFocus() const { return Focus; }

	/** Blocks movement, look and the camera while a screen (talk, board, darkroom) is open. */
	UFUNCTION(BlueprintCallable, Category = "Detective") void SetInputBlocked(bool bBlocked);

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

	UPROPERTY(VisibleAnywhere, Category = "Detective") TObjectPtr<USpringArmComponent> Boom;
	UPROPERTY(VisibleAnywhere, Category = "Detective") TObjectPtr<UCameraComponent> ShoulderCamera;
	UPROPERTY(VisibleAnywhere, Category = "Detective") TObjectPtr<UCameraComponent> ViewfinderCamera;
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
	bool bJogging = false;
	bool bBlocked = false;
};
