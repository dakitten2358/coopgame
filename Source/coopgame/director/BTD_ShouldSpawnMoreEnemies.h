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
	
	UPROPERTY(EditAnywhere, Category=Condition, meta=(ClampMin="0", ClampMax="200"))
	int MaximumEnemyCount;

	virtual bool CalculateRawConditionValue(class UBehaviorTreeComponent& ownerComponent, uint8* nodeMemory) const override;
	
	
};
