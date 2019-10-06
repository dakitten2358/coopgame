// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Camera/PlayerCameraManager.h"
#include "CoopPlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class ACoopPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

	ACoopPlayerCameraManager(const FObjectInitializer& objectInitializer);

	virtual void UpdateCamera(float deltaTime) override;

private:
	float m_currentCrouchOffset;
	float m_maximumCrouchOffset;
	float m_crouchLerpVelocity;

	bool m_wasCrouched;

	float m_defaultCameraOffsetZ;

	float m_normalFov;
	float m_aimingFov;
};
