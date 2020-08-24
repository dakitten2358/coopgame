// Fill out your copyright notice in the Description page of Project Settings.

#include "CoopPlayerCameraManager.h"
#include "NativeCoopCharacter.h"

ACoopPlayerCameraManager::ACoopPlayerCameraManager(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
	, m_wasCrouched(false)
{
	m_normalFov = 90.0f;
	m_aimingFov = 65.0f;

	ViewPitchMin = -80.0f;
	ViewPitchMax = 87.0f;
	bAlwaysApplyModifiers = true;

	// camera delta between standing and crouched
	m_maximumCrouchOffset = 46.0f;

	// HACK. Mirrored from the 3rd person camera offset from CoopCharacter */
	m_defaultCameraOffsetZ = 60.0f;

	/* Ideally matches the transition speed of the character animation (crouch to stand and vice versa) */
	m_crouchLerpVelocity = 12.0f;
}

void ACoopPlayerCameraManager::UpdateCamera(float deltaTime)
{
	ANativeCoopCharacter* myPawn = PCOwner ? Cast<ANativeCoopCharacter>(PCOwner->GetPawn()) : nullptr;
	if (myPawn)
	{
		const float TargetFOV = myPawn->IsAimingDownSights() ? m_aimingFov : m_normalFov;
		DefaultFOV = FMath::FInterpTo(DefaultFOV, TargetFOV, deltaTime, 20.0f);
		SetFOV(DefaultFOV);
	
		// apply smooth camera lerp between crouch toggling
		if (myPawn->bIsCrouched && !m_wasCrouched)
		{
			m_currentCrouchOffset = m_maximumCrouchOffset;
		}
		else if (!myPawn->bIsCrouched && m_wasCrouched)
		{
			m_currentCrouchOffset = -m_maximumCrouchOffset;
		}

		m_wasCrouched = myPawn->bIsCrouched;
		/* Clamp the lerp to 0-1.0 range and interpolate to our new crouch offset */
		m_currentCrouchOffset = FMath::Lerp(m_currentCrouchOffset, 0.0f, FMath::Clamp(m_crouchLerpVelocity * deltaTime, 0.0f, 1.0f));

		FVector CurrentCameraOffset = myPawn->GetCameraComponent()->GetRelativeTransform().GetLocation();
		FVector NewCameraOffset = FVector(CurrentCameraOffset.X, CurrentCameraOffset.Y, m_defaultCameraOffsetZ + m_currentCrouchOffset);
		myPawn->GetCameraComponent()->SetRelativeLocation(NewCameraOffset);
	}

	Super::UpdateCamera(deltaTime);
}