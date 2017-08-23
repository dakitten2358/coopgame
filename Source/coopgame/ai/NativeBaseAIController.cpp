// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "NativeBaseAIController.h"
#include "NativeBaseAICharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "NativeCoopCharacter.h"

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
		m_enemyKeyID = m_blackboardComponent->GetKeyID("TargetToFollow");

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
	UE_LOG(LogCoopGame, Warning, TEXT("AIController unpossessing"));
	Super::UnPossess();

	m_behaviorTreeComponent->StopTree();
}

bool ANativeBaseAIController::FindClosestEnemyWithLOS()
{
	ANativeBaseAICharacter* selfPawn = Cast<ANativeBaseAICharacter>(GetPawn());
	float bestFoundDistanceSquared = MAX_FLT; // std::numeric_limits
	ANativeBaseCharacter* bestFoundEnemy = nullptr;

	if (selfPawn != nullptr)
	{
		auto currentLocation = selfPawn->GetActorLocation();
		for (FConstPawnIterator pawnIterator = GetWorld()->GetPawnIterator(); pawnIterator; ++pawnIterator)
		{
			auto testCharacter = Cast<ANativeCoopCharacter>(*pawnIterator);
			if (testCharacter && testCharacter->IsAlive() /*&& testCharacter->IsEnemyFor(this)*/)
			{
				if (HasWeaponLOSToEnemy(testCharacter))
				{
					auto distanceSquared = (testCharacter->GetActorLocation() - currentLocation).SizeSquared();
					if (distanceSquared < bestFoundDistanceSquared)
					{
						bestFoundDistanceSquared = distanceSquared;
						bestFoundEnemy = testCharacter;
					}
				}
			}
		}

		// did we find an enemy?
		if (bestFoundEnemy != nullptr)
		{
			SetEnemy(bestFoundEnemy);
		}

	}

	return bestFoundEnemy != nullptr;
}

bool ANativeBaseAIController::HasWeaponLOSToEnemy(AActor* enemyActor) const
{
	auto selfPawn = Cast<ANativeBaseAICharacter>(GetPawn());


	bool hasLineOfSight = false;
	// Perform trace to retrieve hit info
	FCollisionQueryParams traceParams(SCENE_QUERY_STAT(AIWeaponLosTrace), true, GetPawn());
	traceParams.bTraceAsyncScene = true;
	traceParams.bReturnPhysicalMaterial = true;

	FVector startLocation = selfPawn->GetActorLocation();
	startLocation.Z += GetPawn()->BaseEyeHeight; //look from eyes

	FHitResult hitResult(ForceInit);
	const FVector endLocation = enemyActor->GetActorLocation();
	GetWorld()->LineTraceSingleByChannel(hitResult, startLocation, endLocation, COLLISION_WEAPON, traceParams);
	if (hitResult.bBlockingHit == true)
	{
		// Theres a blocking hit - check if its our enemy actor
		AActor* hitActor = hitResult.GetActor();
		if (hitActor != nullptr)
		{
			if (hitActor == enemyActor)
			{
				hasLineOfSight = true;
			}
			else if (/*bAnyEnemy == */true)
			{
				/*
				// Its not our actor, maybe its still an enemy ?
				ACharacter* HitChar = Cast<ACharacter>(HitActor);
				if (HitChar != NULL)
				{
					AShooterPlayerState* HitPlayerState = Cast<AShooterPlayerState>(HitChar->PlayerState);
					AShooterPlayerState* MyPlayerState = Cast<AShooterPlayerState>(PlayerState);
					if ((HitPlayerState != NULL) && (MyPlayerState != NULL))
					{
						if (HitPlayerState->GetTeamNum() != MyPlayerState->GetTeamNum())
						{
							bHasLOS = true;
						}
					}
				}
				*/
			}
		}
	}

	return hasLineOfSight;
}

void ANativeBaseAIController::SetEnemy(APawn* InPawn)
{
	if (m_blackboardComponent)
	{
		m_blackboardComponent->SetValue<UBlackboardKeyType_Object>(m_enemyKeyID, InPawn);
		//SetFocus(InPawn);
	}
}