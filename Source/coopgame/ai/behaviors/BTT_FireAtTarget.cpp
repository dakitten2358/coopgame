// Fill out your copyright notice in the Description page of Project Settings.

#include "BTT_FireAtTarget.h"
#include "ai/NativeBaseAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"

EBTNodeResult::Type UBTT_FireAtTarget::ExecuteTask(UBehaviorTreeComponent& ownerComponent, uint8* memory)
{
	bNotifyTick = 1;
	return EBTNodeResult::InProgress;
}

void UBTT_FireAtTarget::TickTask(UBehaviorTreeComponent& ownerComp, uint8 * nodeMemory, float deltaSeconds)
{
	auto aiController = Cast<ANativeBaseAIController>(ownerComp.GetOwner());
	if (aiController)
	{
		if (auto blackboardComponent = ownerComp.GetBlackboardComponent())
		{
			auto target = Cast<AActor>(blackboardComponent->GetValue<UBlackboardKeyType_Object>(GetSelectedBlackboardKey()));
			if (target)
			{
				aiController->ShootAtTarget(target);
			}
		}
	}
}