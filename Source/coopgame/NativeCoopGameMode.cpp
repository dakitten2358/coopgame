// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "NativeCoopGameMode.h"
#include "NativeCoopCharacter.h"
#include "items/NativeWeaponBase.h"

ANativeCoopGameMode::ANativeCoopGameMode(const FObjectInitializer& objectInitializer) : Super(objectInitializer)
{

}

void ANativeCoopGameMode::SetPlayerDefaults(APawn* playerPawn)
{
	Super::SetPlayerDefaults(playerPawn);

	ANativeCoopCharacter* asCharacter = Cast<ANativeCoopCharacter>(playerPawn);
	if (asCharacter != nullptr)
	{
		if (DefaultWeapon != nullptr)
		{
			// spawn the weapon
			FActorSpawnParameters spawnInfo;
			spawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			auto newWeapon = GetWorld()->SpawnActor<ANativeWeaponBase>(DefaultWeapon, spawnInfo);

			// add it to the player
			asCharacter->AddWeapon(newWeapon);
		}
	}
}
