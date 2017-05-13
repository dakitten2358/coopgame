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

	// -----------------------------------------
	// AGameSession
	// -----------------------------------------
public:
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;

	// -----------------------------------------
	// hosting
	// -----------------------------------------
public:
	bool HostSession(TSharedPtr<const FUniqueNetId> userId, FName sessionName, CoopGameType gameType, const FString& mapName, bool isLan, bool isPresence, int32 maxPlayers);

protected:
	// settings used to create the host session
	TSharedPtr<class FCoopGameOnlineSessionSettings> m_hostSettings;

	// delegate for creating a new session
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;

	// callback for new session created
	void OnCreateSessionComplete(FName sessionName, bool wasSuccessful);

	// delegate for starting a new session
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;

	void OnStartOnlineGameComplete(FName sessionName, bool wasSuccessful);

	// -----------------------------------------
	// presence stuff
	// -----------------------------------------
protected:
	// triggered where a presence session is created
	DECLARE_EVENT_TwoParams(ACoopGameSession, FOnCreatePresenceSessionComplete, FName /*sessionName*/, bool /*wasSuccessful*/);
	FOnCreatePresenceSessionComplete CreatePresenceSessionCompleteEvent;

public:
	// get the delegate that's triggered when a presence session is created
	FOnCreatePresenceSessionComplete& OnCreatePresenceSessionComplete() { return CreatePresenceSessionCompleteEvent; }

protected:
	FCoopGameSessionParams m_currentSessionParameters;
	
private:
	typedef ACoopGameSession self_t;
};
