// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"
#include "CoopGameInstance.generated.h"

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
	GENERATED_BODY()

	// ------------------------------------------
	// Game State
	// ------------------------------------------
protected:
	CoopGameState m_currentGameState = CoopGameState::Startup;

public:
	UFUNCTION(BlueprintCallable, Category = "Game State")
	bool IsCurrentState(CoopGameState inState) const;

	UFUNCTION(BlueprintCallable, Category = "Game State")
	CoopGameState TransitionToState(CoopGameState newState);

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void ShowMainMenu();

	// ------------------------------------------
	// Debug Helpers
	// ------------------------------------------
protected:
	void DebugError(const char* fmt, ...) const;

};
