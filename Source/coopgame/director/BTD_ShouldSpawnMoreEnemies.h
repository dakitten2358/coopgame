// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_ShouldSpawnMoreEnemies.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API UBTD_ShouldSpawnMoreEnemies : public UBTDecorator
{
	GENERATED_BODY()
	
	virtual bool CalculateRawConditionValue(class UBehaviorTreeComponent& ownerComponent, uint8* nodeMemory) const override;
};
