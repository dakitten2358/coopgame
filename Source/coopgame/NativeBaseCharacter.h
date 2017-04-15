// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "UnrealNetwork.h"
#include "NativeBaseCharacter.generated.h"

UCLASS()
class COOPGAME_API ANativeBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANativeBaseCharacter(const FObjectInitializer& ObjectInitializer);

	// health
	// ---------------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	bool IsAlive() const;

	// movement
	// ---------------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Movement")
	virtual bool IsSprinting() const;

	virtual void SetSprinting(bool isSprinting);

	float GetSprintingSpeedModifier() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintingSpeedModifier;

	// character wants to run, checked during Tick to see if allowed
	UPROPERTY(Transient, Replicated)
	bool bWantsToSprint;

	// Server side call to update actual sprint state
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetSprinting(bool isSprinting);
	void ServerSetSprinting_Implementation(bool isSprinting);
	bool ServerSetSprinting_Validate(bool isSprinting);

	// aiming
	// ---------------------------------------------------------------
protected:
	void SetAimingDownSights(bool isAiming);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetAimingDownSights(bool isAiming);
	void ServerSetAimingDownSights_Implementation(bool isAiming);
	bool ServerSetAimingDownSights_Validate(bool isAiming);

	UPROPERTY(Transient, Replicated)
	bool bIsAimingDownSights;

	UPROPERTY(EditDefaultsOnly, Category = "Aiming")
	float AimingSpeedModifier;

public:
	UFUNCTION(BlueprintCallable, Category = "Aiming")
	bool IsAimingDownSights() const;

	float GetAimingDownSightsSpeedModifier() const;

	/* Retrieve Pitch/Yaw from current camera */
	UFUNCTION(BlueprintCallable, Category = "Aiming")
	FRotator GetAimOffsets() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Health", Replicated)
	int Health;
};
