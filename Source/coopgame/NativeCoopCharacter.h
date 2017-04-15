// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeBaseCharacter.h"
#include "NativeCoopCharacter.generated.h"

// fwds
class UCharacterMovementComponent;
class UInputComponent;

// native implementation of a player character
UCLASS()
class COOPGAME_API ANativeCoopCharacter : public ANativeBaseCharacter
{
	GENERATED_BODY()

	ANativeCoopCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float deltaTime) override;

	// MOVEMENT
	// -----------------------------------------------------------------------------
public:
	// base turn rate, in deg/sec. other scaling may affect final turn rate.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	// base look up/down rate, in deg/sec. Other scaling may affect final rate.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

protected:
	void MoveForward(float Val);
	void MoveRight(float Val);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	void OnStartCrouching();
	void OnStopCrouching();
	void OnToggleCrouch();

	void OnStartSprinting();
	void OnStopSprinting();
	virtual void SetSprinting(bool isSprinting) override;

private:
	// sets some default values for movement
	void SetupMovementComponentDefaults(UCharacterMovementComponent* movementComponent);

	// AIMING
	// -----------------------------------------------------------------------------
	void OnStartAimingDownSights();
	void OnStopAimingDownSights();


	// CAMERA
	// -----------------------------------------------------------------------------
public:
	// boom to handle distance to player mesh
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* SpringArmForCamera;

	// primary camera of the player
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* CameraComponent;

	FORCEINLINE UCameraComponent* GetCameraComponent()
	{
		return CameraComponent;
	}

private:
	// create and setup the camera and it's associated spring arm
	void CreateAndSetupCamera(const FObjectInitializer& objectInitializer);

	// APawn IMPLEMENTATION
	// -----------------------------------------------------------------------------
protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
