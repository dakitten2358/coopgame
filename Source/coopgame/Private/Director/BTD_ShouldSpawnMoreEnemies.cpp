// Fill out your copyright notice in the Description page of Project Settings.

#include "BTD_ShouldSpawnMoreEnemies.h"
#include "NativeCoopGameMode.h"

bool UBTD_ShouldSpawnMoreEnemies::CalculateRawConditionValue(UBehaviorTreeComponent& ownerComponent, uint8* nodeMemory) const
{
	auto gameMode = GetWorld()->GetAuthGameMode<ANativeCoopGameMode>();
	if (gameMode)
	{
		int currentCount = gameMode->CurrentEnemyCount();
		return currentCount < gameMode->MaxEnemyCount;
	}

	return false;
}