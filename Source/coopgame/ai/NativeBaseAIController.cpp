// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "NativeBaseAIController.h"
#include "NativeBaseAICharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

ANativeBaseAIController::ANativeBaseAIController(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	m_blackboardComponent = objectInitializer.CreateDefaultSubobject<UBlackboardComponent>(this, "BlackboardComponent");
	m_behaviorTreeComponent = objectInitializer.CreateDefaultSubobject<UBehaviorTreeComponent>(this, "BehaviorTreeComponent");
	BrainComponent = m_behaviorTreeComponent;
}

void ANativeBaseAIController::Possess(APawn* pawnToPossess)
{
	Super::Possess(pawnToPossess);

	auto asAICharacter = Cast<ANativeBaseAICharacter>(pawnToPossess);
	if (asAICharacter != nullptr && asAICharacter->AIBehavior != nullptr)
	{
		if (asAICharacter->AIBehavior->BlackboardAsset)
			m_blackboardComponent->InitializeBlackboard(*asAICharacter->AIBehavior->BlackboardAsset);

		// get the keys we're going to need

		// start the behavior
		m_behaviorTreeComponent->StartTree(*(asAICharacter->AIBehavior));
	}
	else
	{
		UE_LOG(LogCoopGame, Warning, TEXT("ANativeBaseAIController possessed a pawn that wasn't a NativeBaseAICharacter or had no AIBehavior specified."));
	}
}

void ANativeBaseAIController::UnPossess()
{
	Super::UnPossess();

	m_behaviorTreeComponent->StopTree();
}
