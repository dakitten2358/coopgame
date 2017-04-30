// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "NativeCoopGameMode.h"
#include "NativeCoopCharacter.h"
#include "items/NativeWeaponBase.h"
#include "ai/NativeBaseAICharacter.h"
#include "ai/NativeBaseAIController.h"
#include "world/NativeEnemyPlayerStart.h"

ANativeCoopGameMode::ANativeCoopGameMode(const FObjectInitializer& objectInitializer) : Super(objectInitializer)
{
	m_maxEnemyCount = 20;
}

void ANativeCoopGameMode::SetPlayerDefaults(APawn* playerPawn)
{
	Super::SetPlayerDefaults(playerPawn);

	ANativeCoopCharacter* asCharacter = Cast<ANativeCoopCharacter>(playerPawn);
	if (asCharacter != nullptr)
	{
		if (DefaultWeapon != nullptr)
		{
			// spawn the weapon
			FActorSpawnParameters spawnInfo;
			spawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			auto newWeapon = GetWorld()->SpawnActor<ANativeWeaponBase>(DefaultWeapon, spawnInfo);

			// add it to the player
			asCharacter->AddWeapon(newWeapon);
		}
	}
}

void ANativeCoopGameMode::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// set up a timer to spawn new enemies
	GetWorldTimerManager().SetTimer(m_timerHandleDefault, this, &ANativeCoopGameMode::OnDefaultTimer, GetWorldSettings()->GetEffectiveTimeDilation(), true);
}

void ANativeCoopGameMode::OnDefaultTimer()
{
	TArray<AActor*> currentEnemies;
	UGameplayStatics::GetAllActorsOfClass(this, ANativeBaseAICharacter::StaticClass(), currentEnemies);
	int currentEnemyCount = currentEnemies.Num();
	if (currentEnemyCount < m_maxEnemyCount)
	{
		int enemiesToSpawn = FMath::Max(m_maxEnemyCount - currentEnemyCount, 0);

		while (enemiesToSpawn > 0)
		{
			SpawnNewEnemy();
			--enemiesToSpawn;
		}
	}
}

void ANativeCoopGameMode::SpawnNewEnemy()
{
	FActorSpawnParameters spawnInfo;
	spawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ANativeBaseAIController *enemyAIController = GetWorld()->SpawnActor<ANativeBaseAIController>(DefaultEnemyController, spawnInfo);
	RestartPlayer(enemyAIController);
}

// Used by RestartPlayer() to determine the pawn to create and possess when a bot or player spawns
UClass* ANativeCoopGameMode::GetDefaultPawnClassForController_Implementation(AController* forController)
{
	if (Cast<ANativeBaseAIController>(forController))
	{
		return DefaultEnemyCharacter;
	}

	return Super::GetDefaultPawnClassForController_Implementation(forController);
}

AActor* ANativeCoopGameMode::ChoosePlayerStart_Implementation(AController* forController)
{
	if (forController->IsA<ANativeBaseAIController>())
	{
		TArray<APlayerStart*> preferredSpawns;
		TArray<APlayerStart*> fallbackSpawns;

		/* Get all playerstart objects in level */
		TArray<AActor*> enemyStarts;
		UGameplayStatics::GetAllActorsOfClass(this, ANativeEnemyPlayerStart::StaticClass(), enemyStarts);

		/* Split the player starts into two arrays for preferred and fallback spawns */
		for (int32 i = 0; i < enemyStarts.Num(); i++)
		{
			ANativeEnemyPlayerStart* startToTest= Cast<ANativeEnemyPlayerStart>(enemyStarts[i]);

			if (startToTest && IsEnemySpawnPointAllowed(startToTest, forController))
			{
				if (IsEnemySpawnPointPreferred(startToTest, forController))
				{
					preferredSpawns.Add(startToTest);
				}
				else
				{
					fallbackSpawns.Add(startToTest);
				}
			}

		}

		/* Pick a random spawnpoint from the filtered spawn points */
		AActor* bestStart = nullptr;
		if (preferredSpawns.Num() > 0)
		{
			bestStart = preferredSpawns[FMath::RandHelper(preferredSpawns.Num())];
		}
		else if (fallbackSpawns.Num() > 0)
		{
			bestStart = fallbackSpawns[FMath::RandHelper(fallbackSpawns.Num())];
		}

		/* If we failed to find any (so BestStart is nullptr) fall back to the base code */
		return bestStart ? bestStart : Super::ChoosePlayerStart_Implementation(forController);
	}

	return Super::ChoosePlayerStart_Implementation(forController);	
}

bool ANativeCoopGameMode::IsEnemySpawnPointAllowed(const ANativeEnemyPlayerStart* spawnPoint, const AController* forController) const
{
	return true;
}

bool ANativeCoopGameMode::IsEnemySpawnPointPreferred(const ANativeEnemyPlayerStart* spawnPoint, const AController* forController) const
{
	return true;
}
