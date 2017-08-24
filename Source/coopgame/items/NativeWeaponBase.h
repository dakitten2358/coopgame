// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "NativeWeaponBase.generated.h"

class ANativeBaseCharacter;

UENUM()
enum class EWeaponState
{
	Idle,
	Firing,
	Reloading,
};


UCLASS()
class COOPGAME_API ANativeWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANativeWeaponBase(const FObjectInitializer& objectInitializer);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Weapon Mesh
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// INVENTORY RELATED
	// -----------------------------------------------------------------------------
public:
	void OnEnterInventory(ANativeBaseCharacter*);
	void OnLeaveInventory();

protected:
	/** pawn owner */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_OwningCharacter)
	ANativeBaseCharacter* OwningCharacter;

	UFUNCTION()
	void OnRep_OwningCharacter();

public:
	void SetOwningCharacter(ANativeBaseCharacter* newOwner);
private:
	
	void ResetOwningCharacter();

	void AttachMeshToCharacter();
	void DetachMeshFromCharacter();	

	// PROJECTILE/FLASH/ETC RELATED
	// -----------------------------------------------------------------------------
protected:
	UPROPERTY(EditDefaultsOnly)
	FName MuzzleAttachPoint;

	FVector GetMuzzleLocation() const;
	FVector GetMuzzleDirection() const;

	FVector GetAdjustedAim() const;
	FVector GetCameraDamageStartLocation(const FVector& aimDirection) const;

	// FIRING RELATED
	// -----------------------------------------------------------------------------
private:
	bool bWantsToFire;
	EWeaponState m_weaponState;
	FTimerHandle m_firingTimerHandle;
	float m_lastFireTime;
	float m_timeBetweenShots;
	bool m_isRefiring;
	bool bPlayingFireAnim;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_BurstCounter)
	int32 BurstCounter;

public:
	void StartFire();
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStartFire();
	void ServerStartFire_Implementation();
	bool ServerStartFire_Validate();

	void StopFire();
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStopFire();
	void ServerStopFire_Implementation();
	bool ServerStopFire_Validate();

	virtual void HandleFiring();
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerHandleFiring();
	void ServerHandleFiring_Implementation();
	bool ServerHandleFiring_Validation();

protected:
	FHitResult WeaponTrace(const FVector& traceFrom, const FVector& traceTo) const;

	virtual void FireWeapon();
	virtual void SimulateWeaponFire();
	virtual void StopSimulatingWeaponFire();

public:
	bool CanFire() const;

private:
	void UpdateWeaponState();
	void SetWeaponState(EWeaponState newState);

	void OnBurstStarted();
	void OnBurstFinished();

	UFUNCTION()
	void OnRep_BurstCounter();
};
