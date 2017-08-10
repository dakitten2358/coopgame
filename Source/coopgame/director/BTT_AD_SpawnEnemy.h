// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_AD_SpawnEnemy.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API UBTT_AD_SpawnEnemy : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	virtual void OnGameplayTaskActivated(class UGameplayTask& Task) override;
	
	
};
