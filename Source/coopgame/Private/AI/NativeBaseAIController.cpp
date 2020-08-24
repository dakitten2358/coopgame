// Fill out your copyright notice in the Description page of Project Settings.

#include "NativeBaseAIController.h"
#include "coopgame.h"
#include "NativeBaseAICharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "EngineUtils.h"
#include "NativeCoopCharacter.h"
#include "items/NativeWeaponBase.h"
#include <limits>

ANativeBaseAIController::ANativeBaseAIController(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	m_blackboardComponent = objectInitializer.CreateDefaultSubobject<UBlackboardComponent>(this, "BlackboardComponent");
	m_behaviorTreeComponent = objectInitializer.CreateDefaultSubobject<UBehaviorTreeComponent>(this, "BehaviorTreeComponent");
	BrainComponent = m_behaviorTreeComponent;

	CombatStatus = EAIState::Idle;
	CombatType = EAIClassBehavior::Assault;
}

void ANativeBaseAIController::OnPossess(APawn* pawnToPossess)
{
	Super::OnPossess(pawnToPossess);

	auto asAICharacter = Cast<ANativeBaseAICharacter>(pawnToPossess);
	if (asAICharacter != nullptr && asAICharacter->AIBehavior != nullptr)
	{
		if (asAICharacter->AIBehavior->BlackboardAsset)
			m_blackboardComponent->InitializeBlackboard(*asAICharacter->AIBehavior->BlackboardAsset);

		// get the keys we're going to need
		m_enemyKeyID = m_blackboardComponent->GetKeyID("TargetToFollow");
		m_keyHighestThreat = m_blackboardComponent->GetKeyID("HighestThreatTarget");
		m_keyCombatStatus = m_blackboardComponent->GetKeyID("CombatStatus");
		m_keyCombatType = m_blackboardComponent->GetKeyID("CombatType");

		// set up defaults
		m_blackboardComponent->SetValue<UBlackboardKeyType_Enum>(m_keyCombatStatus, static_cast<UBlackboardKeyType_Enum::FDataType>(CombatStatus));
		m_blackboardComponent->SetValue<UBlackboardKeyType_Enum>(m_keyCombatType, static_cast<UBlackboardKeyType_Enum::FDataType>(CombatType));
		
		// start the behavior
		m_behaviorTreeComponent->StartTree(*(asAICharacter->AIBehavior));

		
	}
	else
	{
		UE_LOG(LogCoopGame, Warning, TEXT("ANativeBaseAIController possessed a pawn that wasn't a NativeBaseAICharacter or had no AIBehavior specified."));
	}
}

void ANativeBaseAIController::OnUnPossess()
{
	UE_LOG(LogCoopGame, Warning, TEXT("AIController unpossessing"));
	Super::OnUnPossess();

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
		for (TActorIterator<ANativeCoopCharacter> pawnIterator(GetWorld()); pawnIterator; ++pawnIterator)
		{
			auto testCharacter = *pawnIterator;
			if (testCharacter->IsAlive() /*&& testCharacter->IsEnemyFor(this)*/)
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

APawn* ANativeBaseAIController::GetEnemy() const
{
	if (m_blackboardComponent)
	{
		return Cast<APawn>(m_blackboardComponent->GetValue<UBlackboardKeyType_Object>(m_enemyKeyID));
	}

	return nullptr;
}

void ANativeBaseAIController::ShootEnemy()
{
	auto selfPawn = Cast<ANativeBaseAICharacter>(GetPawn());
	auto weapon = selfPawn ? selfPawn->GetCurrentWeapon() : nullptr;

	if (selfPawn == nullptr || weapon == nullptr)
	{
		UE_LOG(LogCoopGame, Log, TEXT("ANativeBaseAIController::ShootEnemy() has either an incorrect pawn, or doesn't have a weapon."));
		return;
	}

	bool canShoot = false;
	auto enemy = Cast<ANativeBaseCharacter>(GetEnemy());
	if (enemy && enemy->IsAlive() && weapon->CanFire())
	{
		// do we have line of sight?
		if (LineOfSightTo(enemy, selfPawn->GetActorLocation()))
			canShoot = true;
	}

	if (canShoot)
		selfPawn->StartWeaponFire();
	else
		selfPawn->StopWeaponFire();
}

void ANativeBaseAIController::ShootAtTarget(AActor* targetToShootAt)
{
	auto selfPawn = Cast<ANativeBaseAICharacter>(GetPawn());
	auto weapon = selfPawn ? selfPawn->GetCurrentWeapon() : nullptr;

	if (selfPawn == nullptr || weapon == nullptr)
	{
		UE_LOG(LogCoopGame, Log, TEXT("ANativeBaseAIController::ShootEnemy() has either an incorrect pawn, or doesn't have a weapon."));
		return;
	}

	bool canShoot = false;
	if (targetToShootAt && /*enemy->IsAlive() &&*/ weapon->CanFire())
	{
		// do we have line of sight?
		if (LineOfSightTo(targetToShootAt, selfPawn->GetActorLocation()))
			canShoot = true;
	}

	if (canShoot)
		selfPawn->StartWeaponFire();
	else
		selfPawn->StopWeaponFire();
}

void ANativeBaseAIController::AddThreat(class ANativeCoopCharacter* toCharacter, int amount)
{
	// add threat
	if (ThreatTable.Contains(toCharacter))
		ThreatTable[toCharacter] += amount;
	else
		ThreatTable.Add(toCharacter, amount);

	// figure out the highest threat
	auto highestThreat = GetHighestThreat();
	if (highestThreat && m_blackboardComponent)
	{
		// update the blackboard
		m_blackboardComponent->SetValue<UBlackboardKeyType_Object>(m_keyHighestThreat, highestThreat);
	}	
}

bool ANativeBaseAIController::IsThreat(class ANativeCoopCharacter* ch) const
{
	return ThreatTable.Find(ch) != nullptr;
}

ANativeCoopCharacter* ANativeBaseAIController::GetHighestThreat() const
{
	ANativeCoopCharacter* highestThreatCharacter = nullptr;
	int highestThreatAmount = std::numeric_limits<int>::min();

	for (const auto& e : ThreatTable)
	{
		if (e.Value > highestThreatAmount)
		{
			highestThreatCharacter = e.Key;
			highestThreatAmount = e.Value;
		}
	}

	return highestThreatCharacter;
}

void ANativeBaseAIController::OnSawPlayer(class ANativeCoopCharacter* playerSeen)
{
	if (!IsThreat(playerSeen))
		AddThreat(playerSeen, 10);
	BecomeHostileIfNecessary();
}

void ANativeBaseAIController::OnHeardPlayer(class ANativeCoopCharacter* playerHeard)
{
	if (!IsThreat(playerHeard))
		AddThreat(playerHeard, 20);
	BecomeHostileIfNecessary();
}

void ANativeBaseAIController::OnTookDamageFromPlayer(class ANativeCoopCharacter* causedByPlayer, float damageAmount)
{
	AddThreat(causedByPlayer, (int)damageAmount);
	BecomeHostileIfNecessary();
}

void ANativeBaseAIController::BecomeHostileIfNecessary()
{
	if (CombatStatus != EAIState::Hostile)
	{
		CombatStatus = EAIState::Hostile;

		// update the blackboard
		if (m_blackboardComponent)
			m_blackboardComponent->SetValue<UBlackboardKeyType_Enum>(m_keyCombatStatus, static_cast<UBlackboardKeyType_Enum::FDataType>(CombatStatus));
	}
}
