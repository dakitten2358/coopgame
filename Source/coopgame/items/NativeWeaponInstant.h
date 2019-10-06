// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "items/NativeWeaponBase.h"
#include "NativeWeaponInstant.generated.h"

/**
 * 
 */
UCLASS()
class ANativeWeaponInstant : public ANativeWeaponBase
{
	GENERATED_BODY()
	
public:
	virtual void FireWeapon() override;

protected:
	void ProcessInstantHit(const FHitResult& hitResult, const FVector& shootOrigin, const FVector& shootDirection);

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerNotifyHit(const FHitResult hitResult, FVector_NetQuantizeNormal shootDirection);
	void ServerNotifyHit_Implementation(const FHitResult hitResult, FVector_NetQuantizeNormal shootDirection);
	bool ServerNotifyHit_Validate(const FHitResult hitResult, FVector_NetQuantizeNormal shootDirection);

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerNotifyMiss(FVector_NetQuantizeNormal shootDirection);
	void ServerNotifyMiss_Implementation(FVector_NetQuantizeNormal shootDirection);
	bool ServerNotifyMiss_Validate(FVector_NetQuantizeNormal shootDirection);

	void ProcessInstantHitConfirmed(const FHitResult& hitResult, const FVector& shootOrigin, const FVector& shootDirection);
	bool ShouldDealDamage(AActor* toActor) const;
	void DealDamage(const FHitResult& hitResult, const FVector& shootDirection);

	UPROPERTY(Transient, ReplicatedUsing = OnRep_HitLocation)
	FVector HitImpactNotify;

	UFUNCTION()
	void OnRep_HitLocation();

	void SimulateInstantHit(const FVector& hitResult);

	//
	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* TracerFX;

	void SpawnTrailEffects(const FVector& impactPoint);
};
