// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "CoopGameSession.h"
#include "CoopGameOnlineSessionSettings.h"
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
		OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &self_t::OnCreateSessionComplete);

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
			UE_LOG(LogCoopGameOnline, Log, TEXT("starting session %s on server"), *GameSessionName.ToString());
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
			UE_LOG(LogCoopGameOnline, Log, TEXT("Ending session %s on server."), *GameSessionName.ToString());
			sessions->EndSession(GameSessionName);
		}
	}
}

bool ACoopGameSession::HostSession(TSharedPtr<const FUniqueNetId> userId, FName sessionName, CoopGameType gameType, const FString& mapName, bool isLan, bool isPresence, int32 maxPlayers)
{
	auto onlineSubsytem = IOnlineSubsystem::Get();
	if (onlineSubsytem != nullptr)
	{
		m_currentSessionParameters.SessionName = sessionName;
		m_currentSessionParameters.IsLan = isLan;
		m_currentSessionParameters.IsPresenceEnabled = isPresence;
		m_currentSessionParameters.UserId = userId;

		MaxPlayers = maxPlayers;

		auto sessions = onlineSubsytem->GetSessionInterface();
		if (sessions.IsValid() && m_currentSessionParameters.UserId.IsValid())
		{
			FString gameTypeText = gameType == CoopGameType::Adventure ? TEXT("adventure") : TEXT("laststand");
			m_hostSettings = MakeShareable(new FCoopGameOnlineSessionSettings(isLan, isPresence, maxPlayers));
			m_hostSettings->Set(SETTING_GAMEMODE, gameTypeText, EOnlineDataAdvertisementType::ViaOnlineService);
			m_hostSettings->Set(SETTING_MAPNAME, mapName, EOnlineDataAdvertisementType::ViaOnlineService);
			m_hostSettings->Set(SETTING_MATCHING_HOPPER, FString("coop_adventure"), EOnlineDataAdvertisementType::DontAdvertise);
			m_hostSettings->Set(SETTING_MATCHING_TIMEOUT, 60.0f, EOnlineDataAdvertisementType::ViaOnlineService);
			m_hostSettings->Set(SETTING_SESSION_TEMPLATE_NAME, FString("GameSession"), EOnlineDataAdvertisementType::DontAdvertise);
			m_hostSettings->Set(SEARCH_KEYWORDS, CustomMatchKeyword, EOnlineDataAdvertisementType::ViaOnlineService);

			OnCreateSessionCompleteDelegateHandle = sessions->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
			sessions->CreateSession(*m_currentSessionParameters.UserId, m_currentSessionParameters.SessionName, *m_hostSettings);
		}
	}
#if !UE_BUILD_SHIPPING
	else
	{
		// super hacky for development
		UE_LOG(LogCoopGameOnline, Warning, TEXT("ACoopGameSession::HostSession: online subsystem was null, falling back on hack"));
		OnCreatePresenceSessionComplete().Broadcast(GameSessionName, true);
		return true;
	}
#endif
	return false;
}

void ACoopGameSession::OnCreateSessionComplete(FName sessionName, bool wasSuccessful)
{
	UE_LOG(LogCoopGameOnline, Verbose, TEXT("ACoopGameSession::OnCreateSessionComplete: sessionName = %s, wasSuccessful = %s"), *sessionName.ToString(), wasSuccessful ? TEXT("true") : TEXT("false"));
	auto onlineSubsytem = IOnlineSubsystem::Get();
	if (onlineSubsytem != nullptr)
	{
		auto sessions = onlineSubsytem->GetSessionInterface();
		sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
	}

	OnCreatePresenceSessionComplete().Broadcast(sessionName, wasSuccessful);
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