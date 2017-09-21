// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "CoopTypes.h"
#include "NativeBaseAIController.generated.h"

// forwards
class UBlackboardComponent;
class UBehaviorTreeComponent;


UCLASS(config=Game)
class COOPGAME_API ANativeBaseAIController : public AAIController
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
	virtual void Possess(APawn* pawnToPossess) override;
	virtual void UnPossess() override;	

public:
	UFUNCTION(BlueprintCallable, Category = Behavior)
	bool FindClosestEnemyWithLOS();

	UFUNCTION(BlueprintCallable, Category = Behavior)
	void ShootEnemy();

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

	UFUNCTION(BlueprintCallable, Category = Behavior)
	class ANativeCoopCharacter* GetHighestThreat() const;

	void BecomeHostileIfNecessary();

public:
	void OnSawPlayer(class ANativeCoopCharacter* playerSeen);
	void OnHeardPlayer(class ANativeCoopCharacter* playerHeard);
	void OnTookDamageFromPlayer(class ANativeCoopCharacter* causedByPlayer, float damageAmount);
};
