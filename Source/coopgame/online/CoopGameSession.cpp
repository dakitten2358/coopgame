// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "CoopGameSession.h"

ACoopGameSession::ACoopGameSession(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{

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

		}

	}
#if !UE_BUILD_SHIPPING
	else
	{
		// super hacky for development
		UE_LOG(LogCoopGameOnline, Warning, TEXT("ACoopGameSession::HostSession: online subsystem was null, falling back on hack"));
		OnCreatePresenceSessionComplete().Broadcast(GameSessionName, true);
	}
#endif
	return true;
}


