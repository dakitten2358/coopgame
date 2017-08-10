// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "BTT_AD_SpawnEnemy.h"
#include "NativeCoopGameMode.h"

void UBTT_AD_SpawnEnemy::OnGameplayTaskActivated(class UGameplayTask& Task)
{
	auto gameMode = GetWorld()->GetAuthGameMode<ANativeCoopGameMode>();
	if (gameMode)
	{
		UE_LOG(LogCoopGame, Warning, TEXT("UBTT_AD_SpawnEnemy::OnGameplayTaskActivated: Spawning new enemy"));	
		gameMode->SpawnNewEnemy();		
	}
}
