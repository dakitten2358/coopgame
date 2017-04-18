// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "NativeWeaponBase.h"
#include "NativeCoopCharacter.h"

void nbw_output(const char* fmt, ...)
{
	static const auto displayDuration = 3.0f;
	static const auto forceAlwaysDisplay = -1;

	if (GEngine != nullptr)
	{
		char buffer[256];

		va_list args;
		va_start(args, fmt);
		vsprintf(buffer, fmt, args);
		va_end(args);

		GEngine->AddOnScreenDebugMessage(forceAlwaysDisplay, displayDuration, FColor::Yellow, buffer);
	}
}


// Sets default values
ANativeWeaponBase::ANativeWeaponBase(const FObjectInitializer& objectInitializer)
{
	Mesh = objectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("WeaponMesh3P"));

	Mesh->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	Mesh->bReceivesDecals = true;
	Mesh->CastShadow = true;
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	RootComponent = Mesh;

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	SetReplicates(true);
	bNetUseOwnerRelevancy = true;
}

void ANativeWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}


// Called when the game starts or when spawned
void ANativeWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANativeWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// INVENTORY RELATED
// -----------------------------------------------------------------------------
void ANativeWeaponBase::OnEnterInventory(ANativeCoopCharacter* ofCharacter)
{
	SetOwningCharacter(ofCharacter);
	AttachMeshToCharacter();
}

void ANativeWeaponBase::OnLeaveInventory()
{
	if (Role == ROLE_Authority)
		ResetOwningCharacter();

	//if (IsEquippedByPawn())
		//;

	DetachMeshFromCharacter();
}

void ANativeWeaponBase::SetOwningCharacter(ANativeCoopCharacter* newOwner)
{
	if (OwningCharacter != newOwner)
	{
		Instigator = newOwner;
		OwningCharacter = newOwner;

		// Net owner for RPC calls.
		SetOwner(newOwner);
	}
}

void ANativeWeaponBase::ResetOwningCharacter()
{
	SetOwningCharacter(nullptr);
}

void ANativeWeaponBase::OnRep_OwningCharacter()
{
	if (OwningCharacter != nullptr)
		OnEnterInventory(OwningCharacter);
	else
		OnLeaveInventory();
}

void ANativeWeaponBase::AttachMeshToCharacter()
{
	if (OwningCharacter != nullptr)
	{
		// Remove and hide
		DetachMeshFromCharacter();

		USkeletalMeshComponent* characterMesh = OwningCharacter->GetMesh();
		FName AttachPoint(TEXT("WeaponSocket"));
		Mesh->SetHiddenInGame(false);
		auto attached = Mesh->AttachToComponent(characterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachPoint);
		if (attached)
			nbw_output("weapon is attached.");
		else
			nbw_output("weapon failed to attach");
	}
}


void ANativeWeaponBase::DetachMeshFromCharacter()
{
	Mesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	Mesh->SetHiddenInGame(true);
}
