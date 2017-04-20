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

	m_timeBetweenShots = 0.5f;
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

void ANativeWeaponBase::StartFire()
{
	nbw_output("ANativeWeaponBase::StartFire()");
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
	nbw_output("ANativeWeaponBase::ServerStartFire_Implementation()");
	StartFire();
}

void ANativeWeaponBase::StopFire()
{
	nbw_output("ANativeWeaponBase::StopFire()");
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
	nbw_output("ANativeWeaponBase::ServerStopFire_Implementation()");
	StopFire();
}

void ANativeWeaponBase::UpdateWeaponState()
{
	EWeaponState newState = EWeaponState::Idle;

	if (bWantsToFire && CanFire())
		newState = EWeaponState::Firing;

	SetWeaponState(newState);
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
	if (previousState == EWeaponState::Firing && newState == EWeaponState::Firing)
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
	nbw_output("ANativeWeaponBase::OnBurstStarted()");
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
	nbw_output("ANativeWeaponBase::OnBurstFinished()");
	// reset the burst counter ??
	// BurstCounter = 0; //??

	// for everyone that's not the dedicated server, stop the firing
	if (GetNetMode() != NM_DedicatedServer)
	{
		//StopSimulatingWeaponFire();
	}

	// clear our timer
	GetWorldTimerManager().ClearTimer(m_firingTimerHandle);
	m_isRefiring = false;
}

void ANativeWeaponBase::HandleFiring()
{
	nbw_output("ANativeWeaponBase::HandleFiring()");
	// can we fire?
	if (CanFire())
	{
		// for everyone that's not the dedicated server, start firing
		//if (GetNetMode() != NM_DedicatedServer)
		//	SimulatingWeaponFire();

		// is it me?
		if (OwningCharacter && OwningCharacter->IsLocallyControlled())
		{
			nbw_output("pew pew");
			// actually fire the weapon
			//FireWeapon(); // derived

			// update the burst counter
			//BurstCounter++;
		}
	}
	else if (OwningCharacter && OwningCharacter->IsLocallyControlled())
	{
		// stop weapon fire fx, but stay in firing state
		//if (BurstCounter > 0)
			//BurstCounter = 0;
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
	nbw_output("ANativeWeaponBase::ServerHandleFiring_Implementation()");
	HandleFiring();

	bool m_shouldUpdateAmmo = (/*CurrentAmmoInClip > 0 && */CanFire());
	if (m_shouldUpdateAmmo)
	{
		// use ammo

		// update fx for remote clients
		//BurstCounter++;
	}
}

bool ANativeWeaponBase::ServerHandleFiring_Validate()
{
	return true;
}
