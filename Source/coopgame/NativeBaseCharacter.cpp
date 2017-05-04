// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "NativeBaseCharacter.h"
#include "NativeCoopGameMode.h"
#include "components/CoopCharacterMovementComponent.h"

// sets default values
ANativeBaseCharacter::ANativeBaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCoopCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// defaults
	Health = 100;
	SprintingSpeedModifier = 2.0f;
	AimingSpeedModifier = 0.5f;

	// don't collide with camera checks to keep 3rd person camera at position other characters are standing behind us
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

// called just before replication
void ANativeBaseCharacter::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	// only repl this property for a short duration after it changes, so join in progress don't get spammed when joining late
	DOREPLIFETIME_ACTIVE_OVERRIDE(ANativeBaseCharacter, LastTakeHitInfo, GetWorld() && GetWorld()->GetTimeSeconds() < LastTakeHitTimeoutTime);
}

// replication
void ANativeBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// only relevant to other clients
	DOREPLIFETIME_CONDITION(ANativeBaseCharacter, bWantsToSprint, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ANativeBaseCharacter, bIsAimingDownSights, COND_SkipOwner);

	// see above
	DOREPLIFETIME_CONDITION(ANativeBaseCharacter, LastTakeHitInfo, COND_Custom);

	// replicate to all clients
	DOREPLIFETIME(ANativeBaseCharacter, Health);
}

// health
// ------------------------------------------------------------
float ANativeBaseCharacter::GetCurrentHealth() const
{
	return Health;
}

float ANativeBaseCharacter::GetMaxHealth() const
{
	// Retrieve the default value of the health property that is assigned on instantiation.
	return GetClass()->GetDefaultObject<ANativeBaseCharacter>()->Health;
}

bool ANativeBaseCharacter::IsAlive() const
{
	return Health > 0;
}

float ANativeBaseCharacter::TakeDamage(float damageAmount, const FDamageEvent& damageEvent, AController* instigator, AActor* damageCauser)
{
	UE_LOG(LogCoopGame, Log, TEXT("holy geez we're taking damage!"));

	// if we're already dead, don't do anything
	if (Health <= 0.0f)
	{
		UE_LOG(LogCoopGame, Log, TEXT("already DEAD, not taking damage."));
		return 0.0f;
	}

	// determine damage based on auth (only authority has the game mode)
	auto authoratitiveGameMode = GetWorld()->GetAuthGameMode<ANativeCoopGameMode>();
	if (authoratitiveGameMode == nullptr)
	{
		UE_LOG(LogCoopGame, Log, TEXT("we're not the authoritay, setting damage to zero"));
		damageAmount = 0.0f;
	}
	
	// let the parent class have a say
	auto actualDamageAmount = Super::TakeDamage(damageAmount, damageEvent, instigator, damageCauser);

	// did we take damage?
	if (actualDamageAmount > 0.0f)
	{
		Health -= actualDamageAmount;
		if (Health <= 0.0f)
			Die(actualDamageAmount, damageEvent, instigator, damageCauser);
		else
			Hit(actualDamageAmount, damageEvent, instigator ? instigator->GetPawn() : nullptr, damageCauser);

		// make a hurt sound in either case
	}
	else
	{
		UE_LOG(LogCoopGame, Log, TEXT("didn't take damage."));
	}

	return actualDamageAmount;
}

bool ANativeBaseCharacter::CanDie(float damageAmount, const FDamageEvent& damageEvent, AController* instigator, AActor* damageCauser) const
{
	// already dying, can't die again!
	if (m_isDying)
		return false;

	// destroyed, waiting to be removed from the game
	if (IsPendingKill())
		return false;

	// only the authoritay can kill us
	if (Role != ROLE_Authority)
		return false;

	// level ending
	#if 0
	auto authGameMode = GetWorld->GetAuthGameMode<ANativeCoopGameMode>();
	if (authGameMode == nullptr || authGameMode->GetMatchState() == MatchState::Leaving)
		return false;
	#endif

	// i guess we can die
	return true;
}

void ANativeBaseCharacter::Die(float damageAmount, const FDamageEvent& damageEvent, AController* instigator, AActor* damageCauser)
{
	UE_LOG(LogCoopGame, Log, TEXT("omg! ded!"));

	// can't die for some reason, bail
	if (!CanDie(damageAmount, damageEvent, instigator, damageCauser))
		return;

	// force health to zero or less
	Health = FMath::Min(0.0f, Health);

	// if we were killed by environmental damage, update the killer
	auto killer = instigator;
	const UDamageType* const damageType = damageEvent.DamageTypeClass ? damageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();
	killer = GetDamageInstigator(killer, *damageType);

	// let the server know who was killed
	auto killedController = (Controller != nullptr) ? Controller : Cast<AController>(GetOwner());
	auto authGameMode = GetWorld()->GetAuthGameMode<ANativeCoopGameMode>();
	#if 0
	authGameMode->Killed(killer, killedController, this, damageType);
	#endif

	// reset the network update frequency, and force a replication update on the movement component to sync the position
	NetUpdateFrequency = GetDefault<ANativeBaseCharacter>()->NetUpdateFrequency;
	GetCharacterMovement()->ForceReplicationUpdate();

	// let people know that we've been killed
	OnDeath(damageAmount, damageEvent, killer ? killer->GetPawn() : nullptr, damageCauser);
}

void ANativeBaseCharacter::Hit(float damageAmount, const FDamageEvent& damageEvent, APawn* instigatorPawn, AActor* damageCauser)
{
	UE_LOG(LogCoopGame, Log, TEXT("omg! hurt! %.2f / %.2f"), Health, GetMaxHealth());

	// server stuff
	if (Role == ROLE_Authority)
	{
		// make sure this gets replicated to everyone
		ReplicateHit(damageAmount, damageEvent, instigatorPawn, damageCauser, EHitResult::Wounded);
	}

	// physics
	if (damageAmount > 0.0f)
		ApplyDamageMomentum(damageAmount, damageEvent, instigatorPawn, damageCauser);

	// update the hud
}

void ANativeBaseCharacter::Suicide()
{
	KilledBy(this);
}

void ANativeBaseCharacter::KilledBy(const APawn* killer)
{
	if (Role == ROLE_Authority)
	{
		AController* killingController = nullptr;
		if (killer)
			killingController = killer->Controller;

		Die(GetCurrentHealth(), FDamageEvent(UDamageType::StaticClass()), killingController, nullptr);
	}
}

void ANativeBaseCharacter::OnDeath(float damageAmount, const FDamageEvent& damageEvent, APawn* instigatorPawn, AActor* damageCauser)
{
	UE_LOG(LogCoopGame, Log, TEXT("OnDeath!"));

	// already dying, no need for overkill (yet?)
	if (m_isDying)
		return;

	// no need to worry about replication anymore
	bReplicateMovement = false;
	bTearOff = true;

	// keep track of dying
	m_isDying = true;

	// server stuff
	if (Role == ROLE_Authority)
	{
		// make sure this gets replicated to everyone
		ReplicateHit(damageAmount, damageEvent, instigatorPawn, damageCauser, EHitResult::Dead);
	}

	// play dying sound (at location, controller may be null at this point)

	// remove all weapons

	// detach from controller
	DetachFromControllerPendingDestroy();
	// stop all montages

	// low health warning

	// running sounds stop

	// play death anim + switch to ragdoll
	// ------------------------------------------------
	if (GetMesh())
	{
		static FName ragdollCollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetCollisionProfileName(ragdollCollisionProfileName);
	}
	SetActorEnableCollision(true);

	// play the death anim montage
	float deathAnimDuration = 0.0f;

	// enable ragdoll
	if (deathAnimDuration > 0.0f)
	{
		// we need to wait for the death anim to finish, so let's set up a timer and wait
		float ragdollWaitTime = deathAnimDuration - 0.7f; // stop early, so it falls more naturally and doesn't revert to default anim pose

		// enable blend physics so that the bones are properly blended during the montage
		GetMesh()->bBlendPhysics = true;

		// local timer, cuz we don't care
		FTimerHandle ragdollTimerHandle;
		bool isTimerLooping = false;
		GetWorldTimerManager().SetTimer(ragdollTimerHandle, [this](){ SetRagdollPhysics(); }, FMath::Max(0.1f, ragdollWaitTime), isTimerLooping);
	}
	else
	{
		// no anim, so let's just rock the ragdoll physics
		SetRagdollPhysics();
	}

	// disable collisions on the capsule
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void ANativeBaseCharacter::SetRagdollPhysics()
{
	auto isRagdollAllowed = false;

	// if it's about to be destroyed, don't bother
	if (IsPendingKill())
		isRagdollAllowed = false;
	// we need a mesh, and it needs to have a physics asset
	else if ((GetMesh() == nullptr) || (GetMesh()->GetPhysicsAsset() == nullptr))
		isRagdollAllowed = false;
	// should be good to enable ragdoll
	else
	{
		isRagdollAllowed = true;

		// setup physics
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->WakeAllRigidBodies();
		GetMesh()->bBlendPhysics = true;
	}

	// stop and disable movement
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	// no need to tick anymore
	GetCharacterMovement()->SetComponentTickEnabled(false);

	if (!isRagdollAllowed)
	{
		// hide and set short lifespan for removal
		TurnOff();
		SetActorHiddenInGame(true);
		SetLifeSpan(1.0f);
	}
	else
	{
		// let the ragdoll play a bit
		SetLifeSpan(10.0f);
	}
}

void ANativeBaseCharacter::ReplicateHit(float damageTaken, const FDamageEvent& damageEvent, APawn* instigatorPawn, AActor* damageCauser, EHitResult hitResult)
{
	float timeoutTime = GetWorld()->GetTimeSeconds() + 0.5f;

	// same frame?
	const auto& lastDamageEvent = LastTakeHitInfo.GetDamageEvent();
	if ((instigatorPawn == LastTakeHitInfo.InstigatorPawn.Get()) &&
		(lastDamageEvent.DamageTypeClass == LastTakeHitInfo.DamageTypeClass) &&
		(LastTakeHitTimeoutTime == timeoutTime))
	{
		// same frame death hit, ignore it
		if (hitResult == EHitResult::Dead && LastTakeHitInfo.HitResult == EHitResult::Dead)
			return;

		// if we're not dead, let's accumulate the damage
		damageTaken += LastTakeHitInfo.ActualDamageTaken;
	}

	LastTakeHitInfo.ActualDamageTaken = damageTaken;
	LastTakeHitInfo.InstigatorPawn = Cast<ANativeBaseCharacter>(instigatorPawn);
	LastTakeHitInfo.DamageCauser = damageCauser;
	LastTakeHitInfo.SetDamageEvent(damageEvent);
	LastTakeHitInfo.HitResult = hitResult;
	LastTakeHitInfo.ForceReplication();

	// update the timeout
	LastTakeHitTimeoutTime = timeoutTime;
}

void ANativeBaseCharacter::OnRep_LastTakeHitInfo()
{
	if (LastTakeHitInfo.HitResult == EHitResult::Dead)
		OnDeath(LastTakeHitInfo.ActualDamageTaken, LastTakeHitInfo.GetDamageEvent(), LastTakeHitInfo.InstigatorPawn.Get(), LastTakeHitInfo.DamageCauser.Get());
	else
		Hit(LastTakeHitInfo.ActualDamageTaken, LastTakeHitInfo.GetDamageEvent(), LastTakeHitInfo.InstigatorPawn.Get(), LastTakeHitInfo.DamageCauser.Get());
}

// movement
// ---------------------------------------------------------------
bool ANativeBaseCharacter::IsSprinting() const
{
	if (GetCharacterMovement() == nullptr)
		return false;

	// have to want to be sprinting, not targeting, and moving
	bool wantsToSprintAndMoving = bWantsToSprint && !IsAimingDownSights() && !GetVelocity().IsZero();
		
	// Don't allow sprint while strafing sideways or standing still (1.0 is straight forward, -1.0 is backward 
	// while near 0 is sideways or standing still)
	//
	// Changing this value to 0.1 allows for diagonal sprinting. (holding W+A or W+D keys)
	bool notStrafing = FVector::DotProduct(GetVelocity().GetSafeNormal2D(), GetActorRotation().Vector()) > 0.8;

	return wantsToSprintAndMoving && notStrafing;
}

void ANativeBaseCharacter::SetSprinting(bool isSprinting)
{
	bWantsToSprint = isSprinting;

	// if we're crouched (from ACharacter), uncrouch!
	if (bIsCrouched)
		UnCrouch();

	// if we're not the authoritay, let the server know we'd like to start sprinting
	if (Role < ROLE_Authority)
		ServerSetSprinting(isSprinting);
}

float ANativeBaseCharacter::GetSprintingSpeedModifier() const
{
	return SprintingSpeedModifier;
}

void ANativeBaseCharacter::ServerSetSprinting_Implementation(bool isSprinting)
{
	SetSprinting(isSprinting);
}

bool ANativeBaseCharacter::ServerSetSprinting_Validate(bool isSprinting)
{
	return true; // no validation
}

// aiming
// ---------------------------------------------------------------
void ANativeBaseCharacter::SetAimingDownSights(bool isAiming)
{
	bIsAimingDownSights = isAiming;

	if (Role < ROLE_Authority)
	{
		ServerSetAimingDownSights(isAiming);
	}
}

void ANativeBaseCharacter::ServerSetAimingDownSights_Implementation(bool isAiming)
{
	SetAimingDownSights(isAiming);
}

bool ANativeBaseCharacter::ServerSetAimingDownSights_Validate(bool isAiming)
{
	return true;
}

bool ANativeBaseCharacter::IsAimingDownSights() const
{
	return bIsAimingDownSights;
}

float ANativeBaseCharacter::GetAimingDownSightsSpeedModifier() const
{
	return AimingSpeedModifier;
}

FRotator ANativeBaseCharacter::GetAimOffsets() const
{
	const FVector AimDirWS = GetBaseAimRotation().Vector();
	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	const FRotator AimRotLS = AimDirLS.Rotation();

	return AimRotLS;
}
