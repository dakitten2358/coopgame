// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTS_AI_ShootEnemy.generated.h"

/**
 * 
 */
UCLASS()
class UBTS_AI_ShootEnemy : public UBTService
{
	GENERATED_BODY()
	
public:
	virtual void TickNode(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory, float deltaSeconds);
	
	
};
