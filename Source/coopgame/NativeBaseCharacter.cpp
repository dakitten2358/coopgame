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

// replication
void ANativeBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// only relevant to other clients
	DOREPLIFETIME_CONDITION(ANativeBaseCharacter, bWantsToSprint, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ANativeBaseCharacter, bIsAimingDownSights, COND_SkipOwner);

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
		{
			// die
			UE_LOG(LogCoopGame, Log, TEXT("omg! ded!"));
			Die(actualDamageAmount, damageEvent, instigator, damageCauser);
		}
		else
		{
			// hit
			UE_LOG(LogCoopGame, Log, TEXT("omg! hurt! %.2f / %.2f"), Health, GetMaxHealth());
		}

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
	#if 0
	OnDeath(damageAmount, damageEvent, killer ? killer->GetPawn() : nullptr, damageCauser);
	#endif
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
