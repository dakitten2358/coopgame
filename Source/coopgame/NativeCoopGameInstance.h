// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"
#include <OnlineIdentityInterface.h>
#include <OnlineSessionInterface.h>
#include "CoopTypes.h"

#include "NativeCoopGameInstance.generated.h"

class FVariantData;
class UUserWidget;
class ACoopGameSession;

/**
 * 
 */
UCLASS(BluePrintable)
class UNativeCoopGameInstance : public UGameInstance
{
	GENERATED_UCLASS_BODY()

private:
	typedef UNativeCoopGameInstance self_t;

	// ------------------------------------------
	// UGameInstance
	// ------------------------------------------
public:
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void StartGameInstance() override;

private:
	FTickerDelegate m_tickDelegate;
	FDelegateHandle m_tickDelegateHandle;

	bool Tick(float deltaInSeconds);

private:
	// general app
	void HandleApplicationWillDeactivate();
	void HandleApplicationSuspend();
	void HandleApplicationResume();
	void HandleApplicationLicenseChange();	

	void HandleSafeFrameChanged();
	void HandleControllerConnectionChange(bool isConnection, int32 unused, int32 gameUserIndex);
	
	// online + multiplayer
	void HandleSignInChangeMessaging();
	void HandleUserLoginChanged(int32 gameUserIndex, ELoginStatus::Type previousLoginStatus, ELoginStatus::Type newLoginStatus, const FUniqueNetId& userID);
	void HandleControllerPairingChanged(int32 gameUserIndex, const FUniqueNetId& previousUser, const FUniqueNetId& newUser);
	void RemoveExistingLocalPlayer(class ULocalPlayer* localPlayer);
	bool IsLocalPlayerOnline(class ULocalPlayer* localPlayer);
	void HandleNetworkConnectionStatusChanged(EOnlineServerConnectionStatus::Type previousConnectionStatus, EOnlineServerConnectionStatus::Type newConnectionStatus);
	void HandleSessionFailure(const FUniqueNetId& newId, ESessionFailure::Type failureType);

	// map
	void OnPostLoadMapWithWorld(class UWorld* world);

	FString m_travelUrl;

	// ------------------------------------------
	// Online
	//-------------------------------------------
private:
	bool m_isOnline;
	bool m_isLicensed;

public:
	UFUNCTION(BlueprintPure, Category = "Online")
	bool IsOnline() const;
	void SetIsOnline(bool isOnline);

	// ------------------------------------------
	// Helpers
	// ------------------------------------------
public:
	UFUNCTION(BlueprintCallable, Category="Misc")
	void SetMouseCursorEnabled(APlayerController* forController, bool isEnabled);
private:
	template <typename TFunc>
	void foreach_localplayer(TFunc f)
	{
		for(auto localPlayerIndex = 0; localPlayerIndex < LocalPlayers.Num(); ++localPlayerIndex)
		{
			auto localPlayer = LocalPlayers[localPlayerIndex];
			f(localPlayerIndex, localPlayer);
		}
	}

	ACoopGameSession* GetGameSession() const;

	// ------------------------------------------
	// Debug Helpers
	// ------------------------------------------
};
