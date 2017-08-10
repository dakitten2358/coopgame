// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "NativeCoopGameMode.generated.h"

class ANativeWeaponBase;
class ANativeBaseAIController;
class ANativeEnemyPlayerStart;
class ANativeBaseAICharacter;
class ANativeCoopGameSession;

/**
 * 
 */
UCLASS()
class COOPGAME_API ANativeCoopGameMode : public AGameMode
{
	GENERATED_UCLASS_BODY()
	
protected:
	virtual void SetPlayerDefaults(APawn* playerPawn) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* forController) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* forController) override;

	// default weapon to spawn with
	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ANativeWeaponBase> DefaultWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Enemies")
	TSubclassOf<ANativeBaseAIController> DefaultEnemyController;

	UPROPERTY(EditDefaultsOnly, Category = "Enemies")
	TSubclassOf<ANativeBaseAICharacter> DefaultEnemyCharacter;

	// for ai controller
	UPROPERTY(transient)
	class UBlackboardComponent* m_blackboardComponent;

	UPROPERTY(transient)
	class UBehaviorTreeComponent* m_behaviorTreeComponent;

	UPROPERTY(EditAnywhere, Category = Behavior)
	class UBehaviorTree* AIBehavior;

	// 
	virtual TSubclassOf<AGameSession> GetGameSessionClass() const override;

public:
	UPROPERTY(EditAnywhere, Category="Game Mode", meta=(ClampMin="0", ClampMax="200"))
	int MaxEnemyCount;

public:
	virtual void PostInitializeComponents() override;

public:
	// enemy spawning
	void SpawnNewEnemy();

	UFUNCTION(BlueprintCallable, Category = "Game State")
	int CurrentEnemyCount() const;

private:
	bool IsEnemySpawnPointAllowed(const ANativeEnemyPlayerStart* spawnPoint, const AController* forController) const;
	bool IsEnemySpawnPointPreferred(const ANativeEnemyPlayerStart* spawnPoint, const AController* forController) const;
};
