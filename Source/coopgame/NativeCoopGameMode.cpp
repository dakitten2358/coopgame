// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "NativeCoopGameMode.h"
#include "NativeCoopCharacter.h"
#include "NativeCoopPlayerController.h"
#include "items/NativeWeaponBase.h"
#include "ai/NativeBaseAICharacter.h"
#include "ai/NativeBaseAIController.h"
#include "world/NativeEnemyPlayerStart.h"
#include "online/CoopGamePlayerState.h"
#include "online/CoopGameSession.h"
#include "online/NativeCoopGameState.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

ANativeCoopGameMode::ANativeCoopGameMode(const FObjectInitializer& objectInitializer) : Super(objectInitializer)
{
	MaxEnemyCount = 10;

	PlayerStateClass = ACoopGamePlayerState::StaticClass();

	m_blackboardComponent = objectInitializer.CreateDefaultSubobject<UBlackboardComponent>(this, "BlackboardComponent");
	m_behaviorTreeComponent = objectInitializer.CreateDefaultSubobject<UBehaviorTreeComponent>(this, "BehaviorTreeComponent");
	AIBehavior = nullptr;

	bDelayedStart = true;
	bStartPlayersAsSpectators = true;
}

void ANativeCoopGameMode::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// set up a simple 1s timer
	auto& timerManager = GetWorldTimerManager();
	timerManager.SetTimer(m_defaultTimerHandle, this, &ANativeCoopGameMode::OnDefaultTimer, GetWorldSettings()->GetEffectiveTimeDilation(), true);
}

void ANativeCoopGameMode::PostInitializeComponents()
{
	// make sure we call our parent's postinitializecomponents
	AGameMode::PostInitializeComponents();

	// start up the AI director
	if (AIBehavior != nullptr && AIBehavior->BlackboardAsset)
	{
		// initialize the blackboard
		if (m_blackboardComponent->InitializeBlackboard(*AIBehavior->BlackboardAsset))
		{
			// get the keys we're going to need
		}
		else
		{
			UE_LOG(LogCoopGame, Error, TEXT("Failed to initialize the blackboard component"));
		}		
	}
	else
	{
		UE_LOG(LogCoopGame, Error, TEXT("ANativeCoopGameMode had no AIBehavior specified or the AIBehavior doesn't specify a blackboard asset."));
		check(false);
	}
}

void ANativeCoopGameMode::PreLogin(const FString& options, const FString& address, const FUniqueNetIdRepl& uniqueID, FString& errorMessage)
{
	auto gameState = Cast<ANativeCoopGameState>(GameState);
	auto isMatchOver = gameState->HasMatchEnded();
	if (isMatchOver)
	{
		errorMessage = TEXT("Match is over!");
	}
	else
	{
		// GameSession can be NULL if the match is over
		Super::PreLogin(options, address, uniqueID, errorMessage);
	}

	UE_LOG(LogCoopGameOnline, Error, TEXT("ANativeCoopGameMode::PreLogin"));
}

void ANativeCoopGameMode::PostLogin(APlayerController* newPlayer)
{
	Super::PostLogin(newPlayer);

	auto player = Cast<ANativeCoopPlayerController>(newPlayer);
	if (player && IsMatchInProgress())
	{
		/*player->ClientGameStarted();*/
		player->ClientStartOnlineGame();
	}

	UE_LOG(LogCoopGameOnline, Error, TEXT("ANativeCoopGameMode::PostLogin"));
}

void ANativeCoopGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	// start the director when the match actually starts
	m_behaviorTreeComponent->StartTree(*AIBehavior, EBTExecutionMode::Looped);
	UE_LOG(LogCoopGame, Warning, TEXT("ANativeCoopGameMode started behavior tree!"));
}

void ANativeCoopGameMode::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();

	// stop it when the match ends
	m_behaviorTreeComponent->StopTree(EBTStopMode::Safe);
}

TSubclassOf<AGameSession> ANativeCoopGameMode::GetGameSessionClass() const
{
	return ACoopGameSession::StaticClass();
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

void ANativeCoopGameMode::SpawnNewEnemy()
{
	FActorSpawnParameters spawnInfo;
	spawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ANativeBaseAIController *enemyAIController = GetWorld()->SpawnActor<ANativeBaseAIController>(DefaultEnemyController, spawnInfo);
	RestartPlayer(enemyAIController);
}

int ANativeCoopGameMode::CurrentEnemyCount() const
{
	TArray<AActor*> currentEnemies;
	UGameplayStatics::GetAllActorsOfClass(this, ANativeBaseAICharacter::StaticClass(), currentEnemies);
	int currentEnemyCount = currentEnemies.Num();
	return currentEnemyCount;
}

// Used by RestartPlayer() to determine the pawn to create and possess when a bot or player spawns
UClass* ANativeCoopGameMode::GetDefaultPawnClassForController_Implementation(AController* forController)
{
	if (Cast<ANativeBaseAIController>(forController))
	{
		return DefaultEnemyCharacter;
	}
	else if (Cast<ANativeCoopPlayerController>(forController))
	{
		auto playerController = Cast<ANativeCoopPlayerController>(forController);
		if (playerController->PlayerState && Cast<ACoopGamePlayerState>(playerController->PlayerState))
		{
			// do we have a preferred character here?  if so, let's use that.
			auto playerState = Cast<ACoopGamePlayerState>(playerController->PlayerState);
			if (playerState->SelectedCharacter)
				return playerState->SelectedCharacter;

			// otherwise, fallback on the default
			return Super::GetDefaultPawnClassForController_Implementation(forController);
		}

	}

	// unknown controller type, fallback on the default
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
			ANativeEnemyPlayerStart* startToTest = Cast<ANativeEnemyPlayerStart>(enemyStarts[i]);

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
	else
	{
		TArray<APlayerStart*> preferredSpawns;

		/* Get all playerstart objects in level */
		TArray<AActor*> playerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), playerStarts);

		/* Split the player starts into two arrays for preferred and fallback spawns */
		for (int32 i = 0; i < playerStarts.Num(); i++)
		{
			APlayerStart* startToTest = Cast<APlayerStart>(playerStarts[i]);
			bool isEnemySpawn = Cast<ANativeEnemyPlayerStart>(playerStarts[i]) != nullptr;

			if (startToTest && !isEnemySpawn)
			{
				preferredSpawns.Add(startToTest);
			}
		}

		/* Pick a random spawnpoint from the filtered spawn points */
		AActor* bestStart = nullptr;
		if (preferredSpawns.Num() > 0)
		{
			bestStart = preferredSpawns[FMath::RandHelper(preferredSpawns.Num())];
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

void ANativeCoopGameMode::OnDefaultTimer()
{
	// if the game is running, update the time elapsed
	auto matchState = GetMatchState();
	auto gameState = Cast<ANativeCoopGameState>(GameState);
	if (gameState && matchState == MatchState::InProgress)
	{
		gameState->TimeElapsed += 1;
	}
	/*
	// if we're in the editor, don't do anything else, except start the match
	if (GetWorld()->IsPlayInEditor())
	{
		if (GetMatchState() == MatchState::WaitingToStart)
			StartMatch();
		return;
	}
	*/

	// check the cooldowns
	if (gameState && gameState->TimeRemaining > 0)
	{
		gameState->TimeRemaining -= 1;
		if (gameState->TimeRemaining <= 0)
		{
			if (matchState == MatchState::WaitingPostMatch)
			{
				RestartGame();
			}
			else if (matchState == MatchState::WaitingToStart)
			{
				StartMatch();

				// let the (connected) clients know that the match is starting
				for (auto it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
				{
					auto playerController = Cast<ANativeCoopPlayerController>(*it);
					if (playerController)
					{
						playerController->ClientHandleMatchStarting();
					}
				}
			}
		}
	}	
}

void ANativeCoopGameMode::Killed(AController* killer, AController* killedPlayer, APawn* killedPawn, const UDamageType* damageType)
{
	// check to see if all players are dead
	bool allDead = true;
	for (auto it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
	{
		auto playerController = Cast<ANativeCoopPlayerController>(*it);
		if (playerController)
		{
			const ANativeBaseCharacter* character = Cast<ANativeBaseCharacter>(playerController->GetPawn());
			if (character)
			{
				if (character->IsAlive())
					allDead = false;
			}
		}
	}

	if (allDead)
	{
		FinishMatch();
	}	
}

void ANativeCoopGameMode::FinishMatch()
{
	ANativeCoopGameState* const gameState = Cast<ANativeCoopGameState>(GameState);
	if (IsMatchInProgress())
	{
		EndMatch();
		//DetermineMatchWinner();

		// notify players
		for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
		{
			//AShooterPlayerState* PlayerState = Cast<AShooterPlayerState>((*It)->PlayerState);
			//const bool bIsWinner = IsWinner(PlayerState);

			(*It)->GameHasEnded(NULL, false);

			if (Cast<ANativeCoopPlayerController>(*It))
			{
				auto pc = Cast<ANativeCoopPlayerController>(*It);
				pc->ClientHandleMatchEnding();
			}
		}

		// lock all pawns
		// pawns are not marked as keep for seamless travel, so we will create new pawns on the next match rather than
		// turning these back on.
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
		{
			(*It)->TurnOff();
		}

		// set up to restart the match
		gameState->TimeRemaining = gameState->PostMatchTime;
	}
}
