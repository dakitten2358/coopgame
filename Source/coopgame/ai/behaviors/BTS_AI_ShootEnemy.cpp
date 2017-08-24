// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "BTS_AI_ShootEnemy.h"
#include "ai/NativeBaseAIController.h"

void UBTS_AI_ShootEnemy::TickNode(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory, float deltaSeconds)
{
	Super::TickNode(ownerComp, nodeMemory, deltaSeconds);

	auto aiController = Cast<ANativeBaseAIController>(ownerComp.GetOwner());
	if (aiController)
	{
		aiController->ShootEnemy();
	}
}