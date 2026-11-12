#include "HarrowCharacter.h"

#include "CaseInteractComponents.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "PhotoCameraComponent.h"
#include "UObject/ConstructorHelpers.h"

#if !UE_BUILD_SHIPPING
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"

// Screenshot runs and tests have no mouse.
// hm.Raise 1|0 raises or lowers the camera of player 0.
static FAutoConsoleCommandWithWorldAndArgs GRaiseCommand(TEXT("hm.Raise"), TEXT("hm.Raise 1|0: raise the camera to the eye or lower it."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (AHarrowCharacter* Detective = Cast<AHarrowCharacter>(UGameplayStatics::GetPlayerPawn(World, 0)))
		{
			Detective->SetRaised(Args.IsEmpty() || Args[0] != TEXT("0"));
		}
	}));
// hm.Look <pitch> [yaw]: aim the view without a mouse (BugItGo would also switch on ghost mode).
static FAutoConsoleCommandWithWorldAndArgs GLookCommand(TEXT("hm.Look"), TEXT("hm.Look <pitch> [yaw]: set the view rotation of player 0."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
		if (PC && !Args.IsEmpty())
		{
			FRotator View = PC->GetControlRotation();
			View.Pitch = FCString::Atof(*Args[0]);
			View.Yaw = Args.Num() > 1 ? FCString::Atof(*Args[1]) : View.Yaw;
			PC->SetControlRotation(View);
		}
	}));
// hm.Go <x> <y> <z> [yaw]: put player 0 somewhere on foot (BugItGo would leave it flying with no collision).
static FAutoConsoleCommandWithWorldAndArgs GGoCommand(TEXT("hm.Go"), TEXT("hm.Go <x> <y> <z> [yaw]: teleport player 0, walking."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
		APawn* Pawn = PC ? PC->GetPawn() : nullptr;
		if (Pawn && Args.Num() >= 3)
		{
			const FVector At(FCString::Atof(*Args[0]), FCString::Atof(*Args[1]), FCString::Atof(*Args[2]));
			FRotator View = PC->GetControlRotation();
			View.Yaw = Args.Num() > 3 ? FCString::Atof(*Args[3]) : View.Yaw;
			Pawn->TeleportTo(At, FRotator(0.f, View.Yaw, 0.f));
			PC->SetControlRotation(View);
		}
	}));
// hm.Body 1|0: show the full world body to the player instead of the view body (a debug comparison of shadows).
static FAutoConsoleCommandWithWorldAndArgs GBodyCommand(TEXT("hm.Body"), TEXT("hm.Body 1|0: show the world body to player 0."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (ACharacter* Detective = Cast<ACharacter>(UGameplayStatics::GetPlayerPawn(World, 0)))
		{
			const bool bShow = Args.IsEmpty() || FCString::Atoi(*Args[0]) != 0;
			Detective->GetMesh()->SetOwnerNoSee(!bShow);
			for (USkeletalMeshComponent* Other : TInlineComponentArray<USkeletalMeshComponent*>(Detective))
			{
				if (Other != Detective->GetMesh())
				{
					Other->SetVisibility(!bShow);
				}
			}
		}
	}));
#endif

AHarrowCharacter::AHarrowCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/Characters/Detective/SK_CH01_Detective/SkeletalMeshes/SK_CH01_Detective.SK_CH01_Detective"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimClass(TEXT("/Game/Characters/Detective/Anims/ABP_Unarmed_CH01"));
	ViewBody = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ViewBody"));
	ViewBody->SetupAttachment(GetCapsuleComponent());
	for (USkeletalMeshComponent* Body : {GetMesh(), ViewBody.Get()})
	{
		if (MeshAsset.Succeeded())
		{
			Body->SetSkeletalMesh(MeshAsset.Object);
		}
		if (AnimClass.Succeeded())
		{
			Body->SetAnimInstanceClass(AnimClass.Class);
		}
		Body->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -96.f), FRotator(0.f, -90.f, 0.f));
	}
	// Not EFirstPersonPrimitiveType: NvRTX's RTXDI shadow rays skip a WorldSpaceRepresentation, so the body
	// would cast no shadow. A hidden-to-owner mesh stays in the ray tracing scene for shadows and reflections.
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->bCastHiddenShadow = true;
	ViewBody->SetOnlyOwnerSee(true);
	ViewBody->SetCastShadow(false);
	ViewBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ViewBody->bVisibleInRayTracing = false;

	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->bOrientRotationToMovement = false;
	Move->MaxWalkSpeed = WalkSpeed;
	Move->MaxWalkSpeedCrouched = 150.f;
	Move->BrakingDecelerationWalking = 2000.f;
	Move->GetNavAgentPropertiesRef().bCanCrouch = true;
	Move->SetCrouchedHalfHeight(58.f); // 1.16 m: fits the 1.7 m catwalk headroom (LEVEL.md)
	BaseEyeHeight = 68.f; // 1.64 m standing
	CrouchedEyeHeight = 40.f;

	EyeCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("EyeCamera"));
	EyeCamera->SetupAttachment(GetCapsuleComponent());
	EyeCamera->SetRelativeLocation(FVector(EyeForward, 0.f, BaseEyeHeight));
	EyeCamera->bUsePawnControlRotation = true;
	EyeCamera->SetFieldOfView(WalkFOV);

	PhotoCamera = CreateDefaultSubobject<UPhotoCameraComponent>(TEXT("PhotoCamera"));
	PhotoCamera->SetupAttachment(EyeCamera);

	Flashlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Flashlight"));
	Flashlight->SetupAttachment(EyeCamera);
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
	// The player never sees his own head; the world copy keeps it for the shadow and the mirror.
	ViewBody->HideBoneByName(TEXT("head"), EPhysBodyOp::PBO_None);
	UpdateSpeed();
}

void AHarrowCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateView(DeltaSeconds);
	UpdateFocus();
}

float AHarrowCharacter::ViewfinderFOV() const
{
	float Aspect = 16.f / 9.f;
	if (const APlayerController* PC = Cast<APlayerController>(Controller))
	{
		int32 X = 0, Y = 0;
		PC->GetViewportSize(X, Y);
		Aspect = Y > 0 ? static_cast<float>(X) / Y : Aspect;
	}
	return PhotoCamera->HorizontalFOV(Aspect);
}

void AHarrowCharacter::UpdateView(float DeltaSeconds)
{
	// The eye follows the crouch; raising eases the FOV to the lens on the same camera, so nothing cuts.
	const float Target = bRaised ? 1.f : 0.f;
	RaiseAlpha = RaiseSeconds > 0.f ? FMath::FInterpConstantTo(RaiseAlpha, Target, DeltaSeconds, 1.f / RaiseSeconds) : Target;
	const float Ease = FMath::InterpEaseInOut(0.f, 1.f, RaiseAlpha, 2.f);
	EyeCamera->SetFieldOfView(FMath::Lerp(WalkFOV, ViewfinderFOV(), Ease));
	// Looking down, the eye leans out over the chest the way a head does, so the legs and shoes show.
	const float Pitch = Controller ? FRotator::NormalizeAxis(Controller->GetControlRotation().Pitch) : 0.f;
	const float Lean = FMath::Clamp(-Pitch / 90.f, 0.f, 1.f);
	EyeCamera->SetRelativeLocation(FVector(EyeForward + LookDownForward * Lean, 0.f, bIsCrouched ? CrouchedEyeHeight : BaseEyeHeight));
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
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->ViewPitchMin = MinPitch;
		}
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
