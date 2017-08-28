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
	virtual void PreInitializeComponents() override;

	virtual void PreLogin(const FString& options, const FString& address, const FUniqueNetIdRepl& uniqueID, FString& errorMessage) override;
	virtual void PostLogin(class APlayerController* newPlayer) override;

	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;

public:
	// enemy spawning
	void SpawnNewEnemy();

	UFUNCTION(BlueprintCallable, Category = "Game State")
	int CurrentEnemyCount() const;

	// death notifications
	virtual void Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType);

protected:
	FTimerHandle m_defaultTimerHandle;
	void OnDefaultTimer();

	virtual void FinishMatch();

private:
	bool IsEnemySpawnPointAllowed(const ANativeEnemyPlayerStart* spawnPoint, const AController* forController) const;
	bool IsEnemySpawnPointPreferred(const ANativeEnemyPlayerStart* spawnPoint, const AController* forController) const;
};
