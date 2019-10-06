// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "UnrealNetwork.h"
#include "CoopTypes.h"
#include "NativeBaseCharacter.generated.h"

UCLASS()
class ANativeBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANativeBaseCharacter(const FObjectInitializer& ObjectInitializer);

	// called just before replication happens
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	// health
	// ---------------------------------------------------------------
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Health", Replicated)
	float Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	bool m_isDying;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_LastTakeHitInfo)
	FTakeHitInfo LastTakeHitInfo;

	float LastTakeHitTimeoutTime;

public:
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	bool IsAlive() const;

	virtual float TakeDamage(float damageAmount, const struct FDamageEvent& damageEvent, class AController* instigator, class AActor* damageCauser) override;
	virtual bool CanDie(float damageAmount, const struct FDamageEvent& damageEvent, class AController* instigator, class AActor* damageCauser) const;

	// should only be called by server/authority
	virtual void Die(float damageAmount, const struct FDamageEvent& damageEvent, class AController* instigator, class AActor* damageCauser);

	// called on both
	virtual void Hit(float damageAmount, const struct FDamageEvent& damageEvent, class APawn* instigatorPawn, class AActor* damageCauser);

	// server only
	virtual void Suicide();

	// server only
	virtual void KilledBy(const APawn* killer);

protected:
	// client and server
	virtual void OnDeath(float damageAmount, const struct FDamageEvent& damageEvent, class APawn* instigatorPawn, class AActor* damageCauser);

	// enable ragdoll
	void SetRagdollPhysics();

	// replication of hits
	void ReplicateHit(float damageTaken, const struct FDamageEvent& damageEvent, class APawn* instigatorPawn, class AActor* damageCauser, EHitResult hitResult);

	// client side replication of hits
	UFUNCTION()
	void OnRep_LastTakeHitInfo();

	// movement
	// ---------------------------------------------------------------
public:
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

};
