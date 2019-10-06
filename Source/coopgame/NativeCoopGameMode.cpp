// Fill out your copyright notice in the Description page of Project Settings.

#include "NativeCoopGameMode.h"
#include "CoopTypes.h"
#include "coopgame.h"
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
#include "Kismet/GameplayStatics.h"

ANativeCoopGameMode::ANativeCoopGameMode(const FObjectInitializer& objectInitializer) : Super(objectInitializer)
{
	MaxEnemyCount = 10;

	PlayerStateClass = ACoopGamePlayerState::StaticClass();

	m_blackboardComponent = objectInitializer.CreateDefaultSubobject<UBlackboardComponent>(this, "BlackboardComponent");
	m_behaviorTreeComponent = objectInitializer.CreateDefaultSubobject<UBehaviorTreeComponent>(this, "BehaviorTreeComponent");
	AIBehavior = nullptr;

	bDelayedStart = true;
	bStartPlayersAsSpectators = true;
	bUseSeamlessTravel = true;
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
		// sucks, cuz this is like vermintide, oh well :(
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
		// if we're logging into a match that's in progress, we need to let the player know
		// they should be able to select a character

		/*player->ClientGameStarted();*/
		//player->ClientStartOnlineGame();
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
	return Super::GetGameSessionClass();
}

void ANativeCoopGameMode::SetPlayerDefaults(APawn* playerPawn)
{
	Super::SetPlayerDefaults(playerPawn);

	ANativeCoopCharacter* asCharacter = Cast<ANativeCoopCharacter>(playerPawn);
	if (asCharacter != nullptr)
	{
		auto preferredWeapon = GetDefaultWeaponClassForNativeCoopPlayerController(Cast<ANativeCoopPlayerController>(playerPawn->GetController()));
		if (preferredWeapon == nullptr)
			preferredWeapon = DefaultWeapon;

		if (preferredWeapon)
		{
			// spawn the weapon
			FActorSpawnParameters spawnInfo;
			spawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			auto newWeapon = GetWorld()->SpawnActor<ANativeWeaponBase>(preferredWeapon, spawnInfo);

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
		auto pawnClass = GetDefaultPawnClassForNativeCoopPlayerController(Cast<ANativeCoopPlayerController>(forController));
		if (pawnClass == nullptr)
		{
			UE_LOG(LogCoopGameOnline, Error, TEXT("Failed to find a pawn for ANativeCoopPlayerController, falling back on default."));
			pawnClass = Super::GetDefaultPawnClassForController_Implementation(forController);
		}
		return pawnClass;
	}

	// unknown controller type, fallback on the default
	return Super::GetDefaultPawnClassForController_Implementation(forController);
}

UClass* ANativeCoopGameMode::GetDefaultPawnClassForNativeCoopPlayerController(ANativeCoopPlayerController* playerController)
{
	if (CharacterDataTable == nullptr)
	{
		UE_LOG(LogCoopGameOnline, Warning, TEXT("No CharacterDataTable present, falling back on default character."));
		return Super::GetDefaultPawnClassForController_Implementation(playerController);
	}

	if (playerController->PlayerState == nullptr || Cast<ACoopGamePlayerState>(playerController->PlayerState) == nullptr)
	{
		UE_LOG(LogCoopGameOnline, Warning, TEXT("Found ANativeCoopPlayerController that doesn't have an ACoopGamePlayerState."));
		return GetRandomPawnClassForNativeCoopPlayerController(playerController);
	}

	// do we have a preferred character here?  if so, let's use that.
	auto playerState = Cast<ACoopGamePlayerState>(playerController->PlayerState);
	auto selectedCharacter = playerState->SelectedCharacterID;
	auto characterInfo = CharacterDataTable->FindRow<FCharacterInfoRow>(selectedCharacter, TEXT("ANativeCoopGameMode::GetDefaultPawnClassForController"));
	if (characterInfo && characterInfo->Character)
	{
		return characterInfo->Character;
	}
	else
	{
		// select a random character out of the table
		UE_LOG(LogCoopGame, Error, TEXT("ANativeCoopGameMode::GetDefaultPawnClassForController received an invalid name: %s"), *selectedCharacter.ToString());
		return GetRandomPawnClassForNativeCoopPlayerController(playerController);
	}

	return nullptr;
}

UClass* ANativeCoopGameMode::GetRandomPawnClassForNativeCoopPlayerController(ANativeCoopPlayerController* playerController)
{
	for (const auto& p : CharacterDataTable->GetRowNames())
	{
		auto characterInfo = CharacterDataTable->FindRow<FCharacterInfoRow>(p, TEXT("ANativeCoopGameMode::GetRandomPawnClassForNativeCoopPlayerController"));
		if (characterInfo && !IsCharacterInUse(playerController, p))
			return characterInfo->Character;
	}

	return nullptr;
}

TSubclassOf<ANativeWeaponBase> ANativeCoopGameMode::GetDefaultWeaponClassForNativeCoopPlayerController(ANativeCoopPlayerController* playerController)
{
	if (CharacterDataTable == nullptr)
	{
		UE_LOG(LogCoopGameOnline, Warning, TEXT("No CharacterDataTable present, falling back on default wepaon."));
		return DefaultWeapon;
	}

	if (playerController->PlayerState == nullptr || Cast<ACoopGamePlayerState>(playerController->PlayerState) == nullptr)
	{
		UE_LOG(LogCoopGameOnline, Warning, TEXT("Found ANativeCoopPlayerController that doesn't have an ACoopGamePlayerState (2)."));
		return DefaultWeapon;
	}

	// do we have a preferred character here?  if so, let's use that.
	auto playerState = Cast<ACoopGamePlayerState>(playerController->PlayerState);
	auto selectedCharacter = playerState->SelectedCharacterID;
	auto characterInfo = CharacterDataTable->FindRow<FCharacterInfoRow>(selectedCharacter, TEXT("ANativeCoopGameMode::GetDefaultPawnClassForController"));
	if (characterInfo && characterInfo->Character)
	{
		return characterInfo->DefaultPrimaryWeapon;
	}

	// fallback on default 
	return DefaultWeapon;
}

bool ANativeCoopGameMode::IsCharacterInUse(ANativeCoopPlayerController* exceptController, const FName& characterID)
{
	for (auto it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
	{
		auto playerController = Cast<ANativeCoopPlayerController>(*it);
		if (playerController && playerController->PlayerState && Cast<ACoopGamePlayerState>(playerController->PlayerState))
		{
			auto playerState = Cast<ACoopGamePlayerState>(playerController->PlayerState);

			if (!playerState->SelectedCharacterID.IsNone() && playerState->SelectedCharacterID == characterID)
				return true;
		}
	}

	return false;
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
	FCollisionQueryParams traceParams(SCENE_QUERY_STAT(AIWeaponLosTrace), true, spawnPoint);
	traceParams.bReturnPhysicalMaterial = true;

	FVector startLocation = spawnPoint->GetActorLocation();
	//startLocation.Z += GetPawn()->BaseEyeHeight; //look from eyes

	// loop over the player controllers
	for (auto it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
	{
		auto playerController = Cast<ANativeCoopPlayerController>(*it);
		if (playerController && playerController->GetPawn())
		{
			FHitResult hitResult(ForceInit);
			const FVector endLocation = playerController->GetPawn()->GetActorLocation();
			GetWorld()->LineTraceSingleByChannel(hitResult, startLocation, endLocation, COLLISION_WEAPON, traceParams);
			if (hitResult.bBlockingHit == true)
			{
				AActor* hitActor = hitResult.GetActor();
				auto hitPlayer = Cast<ANativeCoopCharacter>(hitActor);
				if (hitPlayer)
					return false;
			}
		}
	}

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
				//RestartGame();
				if (GameSession->CanRestartGame())
				{
					if (GetMatchState() == MatchState::LeavingMap)
					{
						//return;
					}
					else
					{
						// move back to the lobby
						GetWorld()->ServerTravel("LobbyMap", false, false);
					}
				}
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
