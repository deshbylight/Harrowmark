#include "HarrowCharacter.h"

#include "CaseInteractComponents.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "PhotoCameraComponent.h"
#include "UObject/ConstructorHelpers.h"

AHarrowCharacter::AHarrowCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimClass(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
	if (MeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshAsset.Object);
	}
	if (AnimClass.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimClass.Class);
	}
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -96.f), FRotator(0.f, -90.f, 0.f));

	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->bOrientRotationToMovement = true;
	Move->RotationRate = FRotator(0.f, 540.f, 0.f);
	Move->MaxWalkSpeed = WalkSpeed;
	Move->MaxWalkSpeedCrouched = 150.f;
	Move->BrakingDecelerationWalking = 2000.f;
	Move->GetNavAgentPropertiesRef().bCanCrouch = true;
	Move->SetCrouchedHalfHeight(58.f); // 1.16 m: fits the 1.7 m catwalk headroom (LEVEL.md)
	BaseEyeHeight = 64.f;
	CrouchedEyeHeight = 40.f;

	Boom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Boom"));
	Boom->SetupAttachment(RootComponent);
	Boom->TargetArmLength = 240.f;
	Boom->SocketOffset = FVector(0.f, 55.f, 60.f);
	Boom->bUsePawnControlRotation = true;
	Boom->ProbeSize = 14.f;
	Boom->bEnableCameraLag = true;
	Boom->CameraLagSpeed = 14.f;

	ShoulderCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ShoulderCamera"));
	ShoulderCamera->SetupAttachment(Boom, USpringArmComponent::SocketName);
	ShoulderCamera->bUsePawnControlRotation = false;
	ShoulderCamera->SetFieldOfView(ShoulderFOV);

	ViewfinderCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewfinderCamera"));
	ViewfinderCamera->SetupAttachment(RootComponent);
	ViewfinderCamera->SetRelativeLocation(FVector(12.f, 0.f, BaseEyeHeight));
	ViewfinderCamera->bUsePawnControlRotation = true;
	ViewfinderCamera->SetAutoActivate(false);

	PhotoCamera = CreateDefaultSubobject<UPhotoCameraComponent>(TEXT("PhotoCamera"));
	PhotoCamera->SetupAttachment(ViewfinderCamera);

	Flashlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Flashlight"));
	Flashlight->SetupAttachment(ViewfinderCamera);
	Flashlight->SetRelativeLocation(FVector(0.f, 18.f, -22.f));
	Flashlight->SetIntensityUnits(ELightUnits::Candelas);
	Flashlight->SetIntensity(1200.f);
	Flashlight->SetAttenuationRadius(2200.f);
	Flashlight->SetInnerConeAngle(10.f);
	Flashlight->SetOuterConeAngle(24.f);
	Flashlight->SetLightColor(FLinearColor(1.f, 0.92f, 0.8f));
	Flashlight->SetCastShadows(true);
	Flashlight->SetVisibility(false);
}

void AHarrowCharacter::BeginPlay()
{
	Super::BeginPlay();
	UpdateSpeed();
}

void AHarrowCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	ViewfinderCamera->SetRelativeLocation(FVector(12.f, 0.f, bIsCrouched ? CrouchedEyeHeight : BaseEyeHeight));
	UpdateFocus();
}

void AHarrowCharacter::BuildInput()
{
	if (Mapping)
	{
		return;
	}
	auto MakeAction = [this](const TCHAR* Name, EInputActionValueType Type)
	{
		UInputAction* Action = NewObject<UInputAction>(this, Name);
		Action->ValueType = Type;
		return Action;
	};
	MoveAction = MakeAction(TEXT("IA_Move"), EInputActionValueType::Axis2D);
	LookAction = MakeAction(TEXT("IA_Look"), EInputActionValueType::Axis2D);
	RaiseAction = MakeAction(TEXT("IA_Raise"), EInputActionValueType::Boolean);
	ShootAction = MakeAction(TEXT("IA_Shoot"), EInputActionValueType::Boolean);
	UseAction = MakeAction(TEXT("IA_Use"), EInputActionValueType::Boolean);
	FlashlightAction = MakeAction(TEXT("IA_Flashlight"), EInputActionValueType::Boolean);
	CrouchAction = MakeAction(TEXT("IA_Crouch"), EInputActionValueType::Boolean);
	JogAction = MakeAction(TEXT("IA_Jog"), EInputActionValueType::Boolean);
	BoardAction = MakeAction(TEXT("IA_Board"), EInputActionValueType::Boolean);

	Mapping = NewObject<UInputMappingContext>(this, TEXT("IMC_Detective"));
	auto MapMove = [this](FKey Key, bool bSwizzle, bool bNegate)
	{
		FEnhancedActionKeyMapping& M = Mapping->MapKey(MoveAction, Key);
		if (bSwizzle)
		{
			M.Modifiers.Add(NewObject<UInputModifierSwizzleAxis>(Mapping));
		}
		if (bNegate)
		{
			M.Modifiers.Add(NewObject<UInputModifierNegate>(Mapping));
		}
	};
	MapMove(EKeys::W, true, false);
	MapMove(EKeys::S, true, true);
	MapMove(EKeys::D, false, false);
	MapMove(EKeys::A, false, true);
	Mapping->MapKey(LookAction, EKeys::Mouse2D);
	Mapping->MapKey(RaiseAction, EKeys::RightMouseButton);
	Mapping->MapKey(ShootAction, EKeys::LeftMouseButton);
	Mapping->MapKey(UseAction, EKeys::E);
	Mapping->MapKey(FlashlightAction, EKeys::F);
	Mapping->MapKey(CrouchAction, EKeys::C);
	Mapping->MapKey(JogAction, EKeys::LeftShift);
	Mapping->MapKey(BoardAction, EKeys::Tab);
}

void AHarrowCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	BuildInput();
	if (const APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Sub = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Sub->AddMappingContext(Mapping, 0);
		}
	}
}

void AHarrowCharacter::SetupPlayerInputComponent(UInputComponent* Input)
{
	Super::SetupPlayerInputComponent(Input);
	BuildInput();
	UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(Input);
	if (!ensureMsgf(EI, TEXT("Enhanced Input is not the default input component")))
	{
		return;
	}
	EI->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHarrowCharacter::Move);
	EI->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHarrowCharacter::Look);
	EI->BindAction(RaiseAction, ETriggerEvent::Started, this, &AHarrowCharacter::RaiseCamera);
	EI->BindAction(RaiseAction, ETriggerEvent::Completed, this, &AHarrowCharacter::LowerCamera);
	EI->BindAction(ShootAction, ETriggerEvent::Started, this, &AHarrowCharacter::Shoot);
	EI->BindAction(UseAction, ETriggerEvent::Started, this, &AHarrowCharacter::Use);
	EI->BindAction(FlashlightAction, ETriggerEvent::Started, this, &AHarrowCharacter::ToggleFlashlight);
	EI->BindAction(CrouchAction, ETriggerEvent::Started, this, &AHarrowCharacter::ToggleCrouch);
	EI->BindAction(JogAction, ETriggerEvent::Started, this, &AHarrowCharacter::StartJog);
	EI->BindAction(JogAction, ETriggerEvent::Completed, this, &AHarrowCharacter::StopJog);
	EI->BindAction(BoardAction, ETriggerEvent::Started, this, &AHarrowCharacter::Board);
}

void AHarrowCharacter::Move(const FInputActionValue& Value)
{
	if (bBlocked || !Controller)
	{
		return;
	}
	const FVector2D Axis = Value.Get<FVector2D>();
	const FRotator Yaw(0.f, Controller->GetControlRotation().Yaw, 0.f);
	AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::X), Axis.Y);
	AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y), Axis.X);
}

void AHarrowCharacter::Look(const FInputActionValue& Value)
{
	if (bBlocked)
	{
		return;
	}
	const FVector2D Axis = Value.Get<FVector2D>() * (bRaised ? ViewfinderSensitivity : MouseSensitivity);
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(-Axis.Y);
}

void AHarrowCharacter::RaiseCamera()
{
	if (bBlocked || bRaised)
	{
		return;
	}
	bRaised = true;
	float Aspect = 16.f / 9.f;
	if (const APlayerController* PC = Cast<APlayerController>(Controller))
	{
		int32 X = 0, Y = 0;
		PC->GetViewportSize(X, Y);
		Aspect = Y > 0 ? static_cast<float>(X) / Y : Aspect;
	}
	ViewfinderCamera->SetFieldOfView(PhotoCamera->HorizontalFOV(Aspect));
	ShoulderCamera->Deactivate();
	ViewfinderCamera->Activate();
	GetMesh()->SetOwnerNoSee(true);
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	UpdateSpeed();
	OnViewfinder.Broadcast(true);
}

void AHarrowCharacter::LowerCamera()
{
	if (!bRaised)
	{
		return;
	}
	bRaised = false;
	ViewfinderCamera->Deactivate();
	ShoulderCamera->Activate();
	GetMesh()->SetOwnerNoSee(false);
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	UpdateSpeed();
	OnViewfinder.Broadcast(false);
}

void AHarrowCharacter::Shoot()
{
	if (bRaised && !bBlocked)
	{
		PhotoCamera->Shoot();
	}
}

void AHarrowCharacter::Use()
{
	if (!bBlocked && !bRaised && Focus && Focus->CanInteract())
	{
		Focus->Interact();
	}
}

void AHarrowCharacter::ToggleFlashlight()
{
	if (!bBlocked)
	{
		Flashlight->SetVisibility(!Flashlight->IsVisible());
	}
}

void AHarrowCharacter::ToggleCrouch()
{
	if (bBlocked)
	{
		return;
	}
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AHarrowCharacter::StartJog()
{
	bJogging = true;
	UpdateSpeed();
}

void AHarrowCharacter::StopJog()
{
	bJogging = false;
	UpdateSpeed();
}

void AHarrowCharacter::Board()
{
	if (!bRaised)
	{
		OnBoardRequested.Broadcast();
	}
}

void AHarrowCharacter::SetInputBlocked(bool bInBlocked)
{
	bBlocked = bInBlocked;
	if (bBlocked)
	{
		LowerCamera();
	}
}

void AHarrowCharacter::UpdateSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = bRaised ? ViewfinderSpeed : bJogging ? JogSpeed : WalkSpeed;
}

void AHarrowCharacter::UpdateFocus()
{
	Focus = nullptr;
	const APlayerController* PC = Cast<APlayerController>(Controller);
	if (!PC || bRaised || !PC->PlayerCameraManager)
	{
		return;
	}
	const FVector From = PC->PlayerCameraManager->GetCameraLocation();
	const FVector Dir = PC->PlayerCameraManager->GetCameraRotation().Vector();
	const FVector Eye = GetPawnViewLocation();
	const float Reach = FVector::Dist(From, Eye) + 400.f;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(DetectiveFocus), false, this);
	FHitResult Hit;
	if (!GetWorld()->SweepSingleByChannel(Hit, From, From + Dir * Reach, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(12.f), Params))
	{
		return;
	}
	const AActor* Actor = Hit.GetActor();
	UCaseInteractComponent* Candidate = Actor ? Actor->FindComponentByClass<UCaseInteractComponent>() : nullptr;
	if (Candidate && FVector::Dist(Eye, Hit.ImpactPoint) <= Candidate->Range && Candidate->CanInteract())
	{
		Focus = Candidate;
	}
}
