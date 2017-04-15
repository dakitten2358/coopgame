// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "CoopCharacterMovementComponent.h"
#include "NativeCoopCharacter.h"

float UCoopCharacterMovementComponent::GetMaxSpeed() const
{
	// get the base speed from the default movement component
	float max_speed = Super::GetMaxSpeed();

	// adjust speed for sprinting or aiming down sights
	const ANativeCoopCharacter* characterOwner = Cast<ANativeCoopCharacter>(PawnOwner);
	if (characterOwner)
	{
		// slow down during aiming
		if (characterOwner->IsAimingDownSights() && !IsCrouching())
		{
			max_speed *= characterOwner->GetAimingDownSightsSpeedModifier();
		}
		// speed up if sprinting
		else if (characterOwner->IsSprinting())
		{
			max_speed *= characterOwner->GetSprintingSpeedModifier();
		}
	}

	return max_speed;
}
