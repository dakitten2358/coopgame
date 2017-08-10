// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "BTT_AD_SpawnEnemy.h"
#include "NativeCoopGameMode.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

EBTNodeResult::Type UBTT_AD_SpawnEnemy::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto gameMode = GetWorld()->GetAuthGameMode<ANativeCoopGameMode>();
	if (gameMode)
	{
		UE_LOG(LogCoopGame, Warning, TEXT("UBTT_AD_SpawnEnemy::ExecuteTask: Spawning new enemy"));
		gameMode->SpawnNewEnemy();

		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}
