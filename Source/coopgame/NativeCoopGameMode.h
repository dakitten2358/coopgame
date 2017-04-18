// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "NativeCoopGameMode.generated.h"

class ANativeWeaponBase;

/**
 * 
 */
UCLASS()
class COOPGAME_API ANativeCoopGameMode : public AGameMode
{
	GENERATED_UCLASS_BODY()
	
protected:
	virtual void SetPlayerDefaults(APawn* playerPawn) override;

	// default weapon to spawn with
	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ANativeWeaponBase> DefaultWeapon;
};
