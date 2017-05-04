// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"
#include <OnlineIdentityInterface.h>
#include <OnlineSessionInterface.h>

#include "CoopGameInstance.generated.h"

class UUserWidget;

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
class COOPGAME_API UCoopGameInstance : public UGameInstance
{
	GENERATED_UCLASS_BODY()

	// ------------------------------------------
	// UGameInstance
	// ------------------------------------------
public:
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void StartGameInstance() override;

private:
	// general app
	void HandleApplicationWillDeactivate();
	
	// online + multiplayer
	void HandleUserLoginChanged(int32 gameUserIndex, ELoginStatus::Type previousLoginStatus, ELoginStatus::Type newLoginStatus, const FUniqueNetId& userID);
	void HandleSignInChangeMessaging();
	void HandleControllerPairingChanged(int32 gameUserIndex, const FUniqueNetId& previousUser, const FUniqueNetId& newUser);

	void RemoveExistingLocalPlayer(class ULocalPlayer* localPlayer);

	// ------------------------------------------
	// Game State
	// ------------------------------------------
protected:
	const CoopGameState m_defaultState = CoopGameState::MainMenu;
	
	CoopGameState m_currentGameState = CoopGameState::Startup;
	CoopGameState m_pendingGameState = CoopGameState::Startup;
	
	UUserWidget* m_mainMenuWidget = nullptr;
	TSubclassOf<UUserWidget> m_mainMenuTemplate;
	FName m_mainMenuLevel;



public:
	UFUNCTION(BlueprintCallable, Category = "Game State")
	bool IsCurrentState(CoopGameState inState) const;

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void TransitionToState(CoopGameState newState);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Game State")
	void ShowMainMenu();

	UFUNCTION(BlueprintCallable, Category = "Init")
	void SetDefaultsForMainMenu(TSubclassOf<UUserWidget> menuTemplate, FName menuLevel);

	// ------------------------------------------
	// Online
	//-------------------------------------------
private:
	bool m_isOnline;
	bool m_isLicensed;

public:
	UFUNCTION(BlueprintPure, Category = "Online")
	bool IsOnline() const;

	// ------------------------------------------
	// Debug Helpers
	// ------------------------------------------
};
