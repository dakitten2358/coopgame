// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeBaseCharacter.h"
#include "NativeBaseAICharacter.generated.h"

UCLASS()
class COOPGAME_API ANativeBaseAICharacter : public ANativeBaseCharacter
{
	GENERATED_BODY()

public:
	ANativeBaseAICharacter(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(EditAnywhere, Category = Behavior)
	class UBehaviorTree* AIBehavior;

	virtual void PossessedBy(AController* newController) override;

	virtual void Destroyed() override;

	// WEAPONS
	// -----------------------------------------------------------------------------
public:
	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ANativeWeaponBase> DefaultWeapon;

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

	/** get currently equipped weapon */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	ANativeWeaponBase* GetCurrentWeapon() const;


	// FIRING
	// ---------------------------------------------------------------
private:
	bool bWantsToFire;

public:
	void OnStartFire();
	void OnStopFire();

	void StartWeaponFire();
	void StopWeaponFire();
};
