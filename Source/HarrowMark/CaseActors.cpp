#include "CaseActors.h"

#include "Animation/AnimInstance.h"
#include "CaseDefinition.h"
#include "CaseInteractComponents.h"
#include "CaseRules.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "InvestigationSubsystem.h"
#include "PhotoSubjectComponent.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	constexpr float MannequinHeight = 180.f;

	UStaticMesh* BlockMesh()
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
		return Cube.Object;
	}

	void SizeBlock(UStaticMeshComponent* Mesh, const FVector& Size)
	{
		const bool bShow = !Size.IsNearlyZero();
		Mesh->SetVisibility(bShow);
		Mesh->SetCollisionEnabled(bShow ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		if (bShow)
		{
			Mesh->SetRelativeScale3D(Size / 100.f);
		}
	}
}

// ---------------------------------------------------------------- subject

ACaseSubjectActor::ACaseSubjectActor()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetStaticMesh(BlockMesh());
	Subject = CreateDefaultSubobject<UPhotoSubjectComponent>(TEXT("Subject"));
	Subject->SetupAttachment(RootComponent);
}

void ACaseSubjectActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SizeBlock(Mesh, MeshSize);
}

void ACaseSubjectActor::BeginPlay()
{
	SizeBlock(Mesh, MeshSize);
	Super::BeginPlay();
}

// ---------------------------------------------------------------- person

ACasePersonActor::ACasePersonActor()
{
	PrimaryActorTick.bCanEverTick = false;
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->InitCapsuleSize(34.f, 90.f);
	Capsule->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	RootComponent = Capsule;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Capsule);
	Mesh->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -90.f), FRotator(0.f, -90.f, 0.f));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimClass(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
	if (MeshAsset.Succeeded())
	{
		Mesh->SetSkeletalMesh(MeshAsset.Object);
	}
	if (AnimClass.Succeeded())
	{
		Mesh->SetAnimInstanceClass(AnimClass.Class);
	}

	Talk = CreateDefaultSubobject<UTalkComponent>(TEXT("Talk"));
	Talk->SetupAttachment(Capsule);
	Subject = CreateDefaultSubobject<UPhotoSubjectComponent>(TEXT("Subject"));
	Subject->SetupAttachment(Capsule);
	Subject->BoxExtent = FVector(30.f, 30.f, 90.f);
	Subject->bHideOwnerWhenGone = false;
}

void ACasePersonActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Mesh->SetRelativeScale3D(FVector(HeightCm > 0.f ? HeightCm / MannequinHeight : 1.f));
}

void ACasePersonActor::BeginPlay()
{
	Subject->bHideOwnerWhenGone = false;
	Super::BeginPlay();
	if (UInvestigationSubsystem* Inv = UInvestigationSubsystem::Get(this))
	{
		Inv->OnCaseChanged.AddDynamic(this, &ACasePersonActor::Refresh);
	}
	Refresh();
}

void ACasePersonActor::EndPlay(const EEndPlayReason::Type Reason)
{
	if (UInvestigationSubsystem* Inv = UInvestigationSubsystem::Get(this))
	{
		Inv->OnCaseChanged.RemoveDynamic(this, &ACasePersonActor::Refresh);
	}
	Super::EndPlay(Reason);
}

void ACasePersonActor::Refresh()
{
	const UInvestigationSubsystem* Inv = UInvestigationSubsystem::Get(this);
	const UCaseDefinition* Case = Inv ? Inv->GetCase() : nullptr;
	if (!Case)
	{
		return;
	}
	const FCaseState& S = Inv->GetState();
	auto Present = [&](FName Shot) { return FCaseSim::ShotPresent(*Case, S, Shot); };
	bHere = (Nights.IsEmpty() || Nights.Contains(S.Night + 1))
		&& (ShownWithShot.IsNone() || Present(ShownWithShot))
		&& !HiddenWithShots.ContainsByPredicate(Present)
		&& (Subject->ShotId.IsNone() || Present(Subject->ShotId));
	SetActorHiddenInGame(!bHere);
	SetActorEnableCollision(bHere);
}

// ---------------------------------------------------------------- gate

ACaseGateActor::ACaseGateActor()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetStaticMesh(BlockMesh());
	Gate = CreateDefaultSubobject<UCaseGateComponent>(TEXT("Gate"));
	Gate->SetupAttachment(RootComponent);
}

void ACaseGateActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SizeBlock(Mesh, MeshSize);
}

// ---------------------------------------------------------------- drive

ACaseDriveActor::ACaseDriveActor()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetStaticMesh(BlockMesh());
	Drive = CreateDefaultSubobject<UDriveComponent>(TEXT("Drive"));
	Drive->SetupAttachment(RootComponent);
}

void ACaseDriveActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SizeBlock(Mesh, MeshSize);
}

