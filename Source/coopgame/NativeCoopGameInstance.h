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

UENUM(BlueprintType)
enum class CoopGameState : uint8
{
	Startup			UMETA(DisplayName = "Startup"),
	MainMenu		UMETA(DisplayName = "Main Menu"),
	ServerList		UMETA(DisplayName = "Server List"),
	Loading			UMETA(DisplayName = "Loading Screen"),
	Playing			UMETA(DisplayName = "Playing"),
	ErrorMessage	UMETA(DisplayName = "Error Message"),
	Unknown			UMETA(DisplayName = "Unknown"),
};

/**
 * 
 */
UCLASS(BluePrintable)
class COOPGAME_API UNativeCoopGameInstance : public UGameInstance
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
	void HandleUserLoginChanged(int32 gameUserIndex, ELoginStatus::Type previousLoginStatus, ELoginStatus::Type newLoginStatus, const FUniqueNetId& userID);
	void HandleSignInChangeMessaging();
	void HandleControllerPairingChanged(int32 gameUserIndex, const FUniqueNetId& previousUser, const FUniqueNetId& newUser);
	void RemoveExistingLocalPlayer(class ULocalPlayer* localPlayer);
	bool IsLocalPlayerOnline(class ULocalPlayer* localPlayer);
	void HandleNetworkConnectionStatusChanged(EOnlineServerConnectionStatus::Type previousConnectionStatus, EOnlineServerConnectionStatus::Type newConnectionStatus);
	void HandleSessionFailure(const FUniqueNetId& newId, ESessionFailure::Type failureType);

	// map
	void OnPostLoadMap();

	FString m_travelUrl;

	// ------------------------------------------
	// Game State
	// ------------------------------------------
protected:
	const CoopGameState m_defaultState = CoopGameState::MainMenu;
	
	CoopGameState m_currentGameState = CoopGameState::Startup;
	CoopGameState m_pendingGameState = CoopGameState::Startup;
	
	UUserWidget* m_mainMenuWidget = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Config")
	TSubclassOf<UUserWidget> m_mainMenuTemplate;

	UPROPERTY(EditDefaultsOnly, Category="Config")
	FString m_mainMenuLevel;

public:
	UFUNCTION(BlueprintCallable, Category = "Game State")
	bool IsCurrentState(CoopGameState inState) const;

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void TransitionToState(CoopGameState newState);

	UFUNCTION(BlueprintImplementableEvent, Category = "Game State")
	void OnGameStateChanged(CoopGameState previousState, CoopGameState newState);

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

	void SetPresenceForLocalPlayers(const FVariantData& presenceData);

	UFUNCTION(BlueprintCallable, Category = "Online")
	bool HostGame(class ULocalPlayer* localPlayer, CoopGameType gameType, const FString& mapName, int difficulty);

private:
	void OnCreatePresenceSessionComplete(FName sessionName, bool wasSuccessful);
	FDelegateHandle m_onCreatePresenceSessionCompleteDelegateHandle;

	// ------------------------------------------
	// Splitscreen
	// ------------------------------------------
	void EnableSplitscreen();
	void DisableSplitscreen();
	void RemoveSplitscreenPlayers();

	// ------------------------------------------
	// State Management
	// ------------------------------------------
private:
	void BeginCurrentState(CoopGameState stateBeginning);
	void EndCurrentState(CoopGameState stateEnding);

	// ------------------------------------------
	// Helpers
	// ------------------------------------------
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

	bool LoadFrontEndMap(const FString& mapName);
	void SetMouseCursorEnabled(APlayerController* forController, bool isEnabled);

	ACoopGameSession* GetGameSession() const;

	// ------------------------------------------
	// Debug Helpers
	// ------------------------------------------
};
