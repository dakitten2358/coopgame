// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "NativeCoopPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ANativeCoopPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ANativeCoopPlayerController(const FObjectInitializer& ObjectInitializer);

	bool IsGameMenuVisible() const;
	void ShowInGameMenu();

	// -----------------------------------------
	// Online
	// -----------------------------------------
	UFUNCTION(reliable, client)
	void ClientStartOnlineGame();

	UFUNCTION(reliable, client)
	void ClientEndOnlineGame();

private:
	// retry timer handle while waiting for the player state to be replicated
	FTimerHandle m_timerHandleWaitingForPlayerState;

	// convenience
	typedef ANativeCoopPlayerController self_t;	
};
