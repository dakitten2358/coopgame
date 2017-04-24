// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "items/NativeWeaponBase.h"
#include "NativeWeaponProjectile.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ANativeWeaponProjectile : public ANativeWeaponBase
{
	GENERATED_BODY()
	
protected:
	// default weapon to spawn with
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> DefaultProjectile;
	
	virtual void FireWeapon() override;

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerFireProjectile(FVector projectileOrigin, FVector_NetQuantizeNormal shootDirection);
	
};
