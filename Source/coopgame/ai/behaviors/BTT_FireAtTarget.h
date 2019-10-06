// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTT_FireAtTarget.generated.h"

/**
 * 
 */
UCLASS()
class UBTT_FireAtTarget : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& ownerComponent, uint8* memory) override;
	virtual void TickTask(UBehaviorTreeComponent& ownerComp, uint8 * nodeMemory, float deltaSeconds) override;
};
