// Fill out your copyright notice in the Description page of Project Settings.

#include "NativeWeaponBase.h"
#include "coopgame.h"
#include "NativeBaseCharacter.h"
#include "NativeCoopPlayerController.h"

// Sets default values
ANativeWeaponBase::ANativeWeaponBase(const FObjectInitializer& objectInitializer)
{
	Mesh = objectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("WeaponMesh3P"));

	Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
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

	MuzzleAttachPoint = TEXT("Muzzle");

	m_timeBetweenShots = 0.2f;
}

void ANativeWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ANativeWeaponBase, BurstCounter, COND_SkipOwner);
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
void ANativeWeaponBase::OnEnterInventory(ANativeBaseCharacter* ofCharacter)
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

void ANativeWeaponBase::SetOwningCharacter(ANativeBaseCharacter* newOwner)
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
		{
			UE_LOG(LogCoopGameWeapon, Verbose, TEXT("Weapon is attached."));
		}
		else
		{
			UE_LOG(LogCoopGameWeapon, Warning, TEXT("Weaopn failed to attach."));
		}
	}
}

void ANativeWeaponBase::DetachMeshFromCharacter()
{
	Mesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	Mesh->SetHiddenInGame(true);
}

// PROJECTILLE RELATED
// -----------------------------------------------------------------------------
FVector ANativeWeaponBase::GetMuzzleLocation() const
{
	return Mesh->GetSocketLocation(MuzzleAttachPoint);
}

FVector ANativeWeaponBase::GetMuzzleDirection() const
{
	return Mesh->GetSocketRotation(MuzzleAttachPoint).Vector();
}

FVector ANativeWeaponBase::GetAdjustedAim() const
{
	auto playerController = Instigator ? Cast<ANativeCoopPlayerController>(Instigator->Controller) : nullptr;
	FVector finalAimDirection = FVector::ZeroVector;

	if (playerController != nullptr)
	{
		FVector cameraLocation;
		FRotator cameraRotation;
		playerController->GetPlayerViewPoint(cameraLocation, cameraRotation);

		finalAimDirection = cameraRotation.Vector();
	}
	else if (Instigator)
	{
		finalAimDirection = Instigator->GetBaseAimRotation().Vector();
	}

	return finalAimDirection;
}


FVector ANativeWeaponBase::GetCameraDamageStartLocation(const FVector& aimDirection) const
{
	auto playerController = OwningCharacter ? Cast<AController>(OwningCharacter->Controller) : nullptr;
	FVector outStartLocation = FVector::ZeroVector;

	if (playerController != nullptr)
	{
		FRotator unusedRotation;
		playerController->GetPlayerViewPoint(outStartLocation, unusedRotation);

		// Adjust trace so there is nothing blocking the ray between the camera and the pawn, and calculate distance from adjusted start
		outStartLocation = outStartLocation + aimDirection * (FVector::DotProduct((Instigator->GetActorLocation() - outStartLocation), aimDirection));
	}

	return outStartLocation;
}


// FIRING RELATED
// -----------------------------------------------------------------------------
void ANativeWeaponBase::StartFire()
{
	UE_LOG(LogCoopGameWeapon, Verbose, TEXT("ANativeWeaponBase::StartFire()"));
	if (Role < ROLE_Authority)
		ServerStartFire();

	if (!bWantsToFire)
	{
		bWantsToFire = true;
		UpdateWeaponState();
	}
}

bool ANativeWeaponBase::ServerStartFire_Validate()
{
	return true;
}

void ANativeWeaponBase::ServerStartFire_Implementation()
{
	UE_LOG(LogCoopGameWeapon, Verbose, TEXT("ANativeWeaponBase::ServerStartFire_Implementation()"));
	StartFire();
}

void ANativeWeaponBase::StopFire()
{
	UE_LOG(LogCoopGameWeapon, Verbose, TEXT("ANativeWeaponBase::StopFire()"));
	if (Role < ROLE_Authority)
		ServerStopFire();

	if (bWantsToFire)
	{
		bWantsToFire = false;
		UpdateWeaponState();
	}
}

bool ANativeWeaponBase::ServerStopFire_Validate()
{
	return true;
}

void ANativeWeaponBase::ServerStopFire_Implementation()
{
	UE_LOG(LogCoopGameWeapon, Verbose, TEXT("ANativeWeaponBase::ServerStopFire_Implementation()"));
	StopFire();
}

void ANativeWeaponBase::UpdateWeaponState()
{
	EWeaponState newState = EWeaponState::Idle;

	if (bWantsToFire && CanFire())
		newState = EWeaponState::Firing;

	SetWeaponState(newState);
}

EWeaponState ANativeWeaponBase::GetCurrentState() const
{
	return m_weaponState;
}

bool ANativeWeaponBase::CanFire() const
{
	bool characterCanFire = true; // check the pawn to see if it's ok to fire
	bool stateOk = m_weaponState== EWeaponState::Idle || m_weaponState == EWeaponState::Firing;
	return characterCanFire && stateOk;
}

void ANativeWeaponBase::SetWeaponState(EWeaponState newState)
{
	const EWeaponState previousState = m_weaponState;

	// just stopped firing?
	if (previousState == EWeaponState::Firing && newState == EWeaponState::Idle)
	{
		OnBurstFinished();
	}

	m_weaponState = newState;

	// just started firing?
	if (previousState != EWeaponState::Firing && m_weaponState == EWeaponState::Firing)
	{
		OnBurstStarted();
	}
}

void ANativeWeaponBase::OnBurstStarted()
{
	UE_LOG(LogCoopGameWeapon, Verbose, TEXT("ANativeWeaponBase::OnBurstStarted()"));
	float currentTime = GetWorld()->GetTimeSeconds();

	if (m_lastFireTime > 0 && m_timeBetweenShots > 0 && (m_lastFireTime + m_timeBetweenShots) > currentTime)
	{
		GetWorldTimerManager().SetTimer(m_firingTimerHandle, this, &ANativeWeaponBase::HandleFiring, m_lastFireTime + m_timeBetweenShots - currentTime, false);
	}
	else
	{
		HandleFiring();
	}
}

void ANativeWeaponBase::OnBurstFinished()
{
	UE_LOG(LogCoopGameWeapon, Verbose, TEXT("ANativeWeaponBase::OnBurstFinished()"));
	
	// reset the burst counter ??
	BurstCounter = 0;

	// for everyone that's not the dedicated server, stop the firing
	if (GetNetMode() != NM_DedicatedServer)
	{
		StopSimulatingWeaponFire();
	}

	// clear our timer
	GetWorldTimerManager().ClearTimer(m_firingTimerHandle);
	m_isRefiring = false;
}

void ANativeWeaponBase::HandleFiring()
{
	UE_LOG(LogCoopGameWeapon, Verbose, TEXT("ANativeWeaponBase::HandleFiring()"));
	// can we fire?
	if (CanFire())
	{
		UE_LOG(LogCoopGameWeapon, Verbose, TEXT("Firing"));
		// for everyone that's not the dedicated server, start firing
		if (GetNetMode() != NM_DedicatedServer)
			SimulateWeaponFire();

		// is it me?
		if (OwningCharacter && OwningCharacter->IsLocallyControlled())
		{
			FireWeapon();			

			// update the burst counter
			BurstCounter++;
		}
		else
		{
			UE_LOG(LogCoopGameWeapon, Warning, TEXT("not me!"));
		}

	}
	else if (OwningCharacter && OwningCharacter->IsLocallyControlled())
	{
		// stop weapon fire fx, but stay in firing state
		if (BurstCounter > 0)
			BurstCounter = 0;
	}

	if (OwningCharacter && OwningCharacter->IsLocallyControlled())
	{
		// let the server know
		if (Role < ROLE_Authority)
			ServerHandleFiring();

		// shooting another shot?
		m_isRefiring = (m_weaponState == EWeaponState::Firing && m_timeBetweenShots > 0.0f);
		if (m_isRefiring)
			GetWorldTimerManager().SetTimer(m_firingTimerHandle, this, &ANativeWeaponBase::HandleFiring, m_timeBetweenShots, false);
	}
}

void ANativeWeaponBase::ServerHandleFiring_Implementation()
{
	UE_LOG(LogCoopGameWeapon, Verbose, TEXT("ANativeWeaponBase::ServerHandleFiring_Implementation()"));
	HandleFiring();

	bool m_shouldUpdateAmmo = (/*CurrentAmmoInClip > 0 && */CanFire());
	if (m_shouldUpdateAmmo)
	{
		// use ammo

		// update fx for remote clients
		BurstCounter++;
	}
}

bool ANativeWeaponBase::ServerHandleFiring_Validate()
{
	return true;
}

void ANativeWeaponBase::OnRep_BurstCounter()
{
	if (BurstCounter > 0)
	{
		SimulateWeaponFire();
	}
	else
	{
		StopSimulatingWeaponFire();
	}
}

FHitResult ANativeWeaponBase::WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo, const FName& traceTag) const
{
	FCollisionQueryParams TraceParams(TEXT("WeaponTrace"), true, Instigator);
	TraceParams.bReturnPhysicalMaterial = true;
	TraceParams.TraceTag = traceTag;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, TraceFrom, TraceTo, COLLISION_WEAPON, TraceParams);

	return Hit;
}

void ANativeWeaponBase::FireWeapon()
{
	UE_LOG(LogCoopGameWeapon, Display, TEXT("pew pew"));
}

void ANativeWeaponBase::SimulateWeaponFire()
{
	//if (MuzzleFX)
	//{
	//	MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh, MuzzleAttachPoint);
	//}

	if (!bPlayingFireAnim)
	{
		//PlayWeaponAnimation(FireAnim);
		bPlayingFireAnim = true;
	}

	//PlayWeaponSound(FireSound);
}

void ANativeWeaponBase::StopSimulatingWeaponFire()
{
	if (bPlayingFireAnim)
	{
		//StopWeaponAnimation(FireAnim);
		bPlayingFireAnim = false;
	}
}
