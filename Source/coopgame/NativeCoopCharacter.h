// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeBaseCharacter.h"
#include "NativeCoopCharacter.generated.h"

// fwds
class UCharacterMovementComponent;
class UInputComponent;
class ANativeWeaponBase;

// native implementation of a player character
UCLASS()
class COOPGAME_API ANativeCoopCharacter : public ANativeBaseCharacter
{
	GENERATED_BODY()
public:
	ANativeCoopCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float deltaTime) override;
	virtual void Destroyed() override;

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

	// WEAPONS
	// -----------------------------------------------------------------------------
public:
	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWeapon)
	class ANativeWeaponBase* CurrentWeapon;

	void AddWeapon(ANativeWeaponBase* weapon);
	void EquipWeapon(ANativeWeaponBase* Weapon);

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerEquipWeapon(ANativeWeaponBase* Weapon);
	void ServerEquipWeapon_Implementation(ANativeWeaponBase* Weapon);
	bool ServerEquipWeapon_Validate(ANativeWeaponBase* Weapon);

	/* OnRep functions can use a parameter to hold the previous value of the variable. Very useful when you need to handle UnEquip etc. */
	UFUNCTION()
	void OnRep_CurrentWeapon(ANativeWeaponBase* LastWeapon);
	
	void SetCurrentWeapon(ANativeWeaponBase* weapon);

	void DestroyInventory();


	// FIRING
	// ---------------------------------------------------------------
private:
	bool bWantsToFire;

public:
	void OnStartFire();
	void OnStopFire();

	void StartWeaponFire();
	void StopWeaponFire();


	// APawn IMPLEMENTATION
	// -----------------------------------------------------------------------------
protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
