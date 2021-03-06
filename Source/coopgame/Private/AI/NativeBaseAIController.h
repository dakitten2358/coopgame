// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "CoopTypes.h"
#include "NativeBaseAIController.generated.h"

// forwards
class UBlackboardComponent;
class UBehaviorTreeComponent;


UCLASS(config=Game)
class ANativeBaseAIController : public AAIController
{
	GENERATED_UCLASS_BODY()

private:
	UPROPERTY(transient)
	UBlackboardComponent* m_blackboardComponent;

	UPROPERTY(transient)
	UBehaviorTreeComponent* m_behaviorTreeComponent;

	int32 m_enemyKeyID;
	int32 m_keyCombatStatus;
	int32 m_keyCombatType;
	int32 m_keyHighestThreat;	
	
public:
	// AController interface
	virtual void OnPossess(APawn* pawnToPossess) override;
	virtual void OnUnPossess() override;	

public:
	UFUNCTION(BlueprintCallable, Category = Behavior)
	bool FindClosestEnemyWithLOS();

	UFUNCTION(BlueprintCallable, Category = Behavior)
	void ShootEnemy();

	UFUNCTION(BlueprintCallable, Category = Behavior)
	void ShootAtTarget(AActor* targetToShootAt);

private:
	void SetEnemy(class APawn* enemyPawn);
	class APawn* GetEnemy() const;
	bool HasWeaponLOSToEnemy(AActor* enemyActor) const;

	// Threat
	UPROPERTY(EditAnywhere, Category = Behavior)
	EAIState			CombatStatus;

	UPROPERTY(EditAnywhere, Category = Behavior)
	EAIClassBehavior	CombatType;

	TMap<class ANativeCoopCharacter*, int>  ThreatTable;
	void AddThreat(class ANativeCoopCharacter*, int amount);
	bool IsThreat(class ANativeCoopCharacter*) const;

	UFUNCTION(BlueprintCallable, Category = Behavior)
	class ANativeCoopCharacter* GetHighestThreat() const;

	void BecomeHostileIfNecessary();

public:
	void OnSawPlayer(class ANativeCoopCharacter* playerSeen);
	void OnHeardPlayer(class ANativeCoopCharacter* playerHeard);
	void OnTookDamageFromPlayer(class ANativeCoopCharacter* causedByPlayer, float damageAmount);
};
