// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameSession.h"
#include "CoopTypes.h"
#include "Online.h"
#include "CoopGameSession.generated.h"

struct FCoopGameSessionParams
{
	// name of session that the settings are associated with
	FName SessionName;

	// is it a LAN match?
	bool IsLan;

	// presence enabled session?  (public match?)
	bool IsPresenceEnabled;

	// id of the lobby owner
	TSharedPtr<const FUniqueNetId> UserId;

	// search result choice to join
	int32 BestSessionIndex;

	FCoopGameSessionParams()
		: SessionName(NAME_None)
		, IsLan(false)
		, IsPresenceEnabled(false)
		, BestSessionIndex(0)
	{}
};

UCLASS()
class COOPGAME_API ACoopGameSession : public AGameSession
{
	GENERATED_UCLASS_BODY()
	
public:
	bool HostSession(TSharedPtr<const FUniqueNetId> userId, FName sessionName, CoopGameType gameType, const FString& mapName, bool isLan, bool isPresence, int32 maxPlayers);

private:
	FCoopGameSessionParams m_currentSessionParameters;
	
protected:
	// triggered where a presence session is created
	DECLARE_EVENT_TwoParams(ACoopGameSession, FOnCreatePresenceSessionComplete, FName /*sessionName*/, bool /*wasSuccessful*/);
	FOnCreatePresenceSessionComplete CreatePresenceSessionCompleteEvent;

	// get the delegate that's triggered when a presence session is created
	FOnCreatePresenceSessionComplete& OnCreatePresenceSessionComplete() { return CreatePresenceSessionCompleteEvent; }
	
};
