// Fill out your copyright notice in the Description page of Project Settings.

#include "CoopGameSession.h"
#include "coopgame.h"
#include "OnlineSubsystemSessionSettings.h"
#include "NativeCoopPlayerController.h"

namespace
{
	const FString CustomMatchKeyword("CoopgameAdventure");
}

ACoopGameSession::ACoopGameSession(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	// if we're not the default object for this class
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		// set up the delegates
		OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &self_t::OnStartOnlineGameComplete);
	}
}

void ACoopGameSession::HandleMatchHasStarted()
{
	// start online game locally and wait for completion
	auto onlineSubsystem = IOnlineSubsystem::Get();
	if (onlineSubsystem != nullptr)
	{
		auto sessions = onlineSubsystem->GetSessionInterface();
		if (sessions.IsValid())
		{
			UE_LOG(LogCoopGameOnline, Log, TEXT("starting session %s on server"), *((FName)GameSessionName).ToString());
			OnStartSessionCompleteDelegateHandle = sessions->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);
			sessions->StartSession(GameSessionName);
		}
	}
}

void ACoopGameSession::HandleMatchHasEnded()
{
	auto onlineSubsystem = IOnlineSubsystem::Get();
	if (onlineSubsystem != nullptr)
	{
		auto sessions = onlineSubsystem->GetSessionInterface();
		if (sessions.IsValid())
		{
			// tell the clients to end
			for (auto it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
			{
				auto playerController = Cast<ANativeCoopPlayerController>(*it);
				if (playerController && !playerController->IsLocalPlayerController())
				{
					playerController->ClientEndOnlineGame();
				}
			}

			// end the server
			UE_LOG(LogCoopGameOnline, Log, TEXT("Ending session %s on server."), *((FName)GameSessionName).ToString());
			sessions->EndSession(GameSessionName);
		}
	}
}

void ACoopGameSession::OnStartOnlineGameComplete(FName sessionName, bool wasSuccessful)
{
	UE_LOG(LogCoopGameOnline, Verbose, TEXT("ACoopGameSession::OnStartOnlinegameComplete: sessionName = %s, wasSuccessful = %s"), *sessionName.ToString(), wasSuccessful ? TEXT("true") : TEXT("false"));
	auto onlineSubsytem = IOnlineSubsystem::Get();
	if (onlineSubsytem != nullptr)
	{
		auto sessions = onlineSubsytem->GetSessionInterface();
		if (sessions.IsValid())
		{
			sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
		}
	}

	if (wasSuccessful)
	{
		// tell non-local players to start the online game
		for (auto it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
		{
			auto playerController = Cast<ANativeCoopPlayerController>(*it);
			if (playerController && !playerController->IsLocalPlayerController())
			{
				playerController->ClientStartOnlineGame();
			}
		}
	}
}