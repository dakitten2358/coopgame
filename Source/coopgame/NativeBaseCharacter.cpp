// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "NativeBaseCharacter.h"
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
