// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "NativeCoopPlayerController.h"
#include "CoopPlayerCameraManager.h"
#include "online/CoopGamePlayerState.h"
#include "OnlineSubsystem.h"
#include "Online.h"


ANativeCoopPlayerController::ANativeCoopPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// assign the class types we wish to use
	PlayerCameraManagerClass = ACoopPlayerCameraManager::StaticClass();
}

bool ANativeCoopPlayerController::IsGameMenuVisible() const
{
	UE_LOG(LogCoopGameNotImplemented, Error, TEXT("ANativeCoopPlayerController::IsGameMenuVisible()"));
	return false;
}

void ANativeCoopPlayerController::ShowInGameMenu()
{
	UE_LOG(LogCoopGameNotImplemented, Error, TEXT("ANativeCoopPlayerController::ShowInGameMenu"));
}

// -----------------------------------------
// Online
// -----------------------------------------
void ANativeCoopPlayerController::ClientStartOnlineGame_Implementation()
{
	if (!IsPrimaryPlayer())
		return;

	auto playerState = Cast<ACoopGamePlayerState>(PlayerState);
	if (playerState)
	{
		auto onlineSubsystem = IOnlineSubsystem::Get();
		if (onlineSubsystem)
		{
			auto sessions = onlineSubsystem->GetSessionInterface();
			if (sessions.IsValid())
			{
				UE_LOG(LogCoopGameOnline, Log, TEXT("Starting sesion %s on client"), *playerState->SessionName.ToString());
				sessions->StartSession(playerState->SessionName);
			}
		}
	}
	else
	{
		// player state hasn't been replicated yet, retry
		GetWorld()->GetTimerManager().SetTimer(m_timerHandleWaitingForPlayerState, this, &self_t::ClientStartOnlineGame_Implementation, 0.1f, false);
	}
}

void ANativeCoopPlayerController::ClientEndOnlineGame_Implementation()
{
	if (!IsPrimaryPlayer())
		return;

	auto playerState = Cast<ACoopGamePlayerState>(PlayerState);
	if (playerState)
	{
		auto onlineSubsystem = IOnlineSubsystem::Get();
		if (onlineSubsystem)
		{
			auto sessions = onlineSubsystem->GetSessionInterface();
			if (sessions.IsValid())
			{
				UE_LOG(LogCoopGameOnline, Log, TEXT("Ending session %s on client"), *playerState->SessionName.ToString());
				sessions->EndSession(playerState->SessionName);
			}
		}
	}
}
