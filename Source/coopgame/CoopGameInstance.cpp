// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "CoopGameInstance.h"
#include <cstdarg>
#include <cstdio>
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include <Runtime/UMG/Public/UMG.h>

bool UCoopGameInstance::IsCurrentState(CoopGameState inState) const
{
	return m_currentGameState == inState;
}

CoopGameState UCoopGameInstance::TransitionToState(CoopGameState newState)
{
	// tried to transition to the same state
	if (m_currentGameState == newState)
	{
		DebugError("Attempted to transition to the same state (%d -> %d)", m_currentGameState, newState);
		return m_currentGameState;
	}

	// clean up
	switch (m_currentGameState)
	{
	// main menu
	case CoopGameState::MainMenu:
		break;
	// in the case of startup, or any unknown state, do nothing
	case CoopGameState::Startup:
	default:
		break;
	}

	// keep the current state up to date
	m_currentGameState = newState;
	return m_currentGameState;
}

void UCoopGameInstance::SetDefaultsForMainMenu(TSubclassOf<UUserWidget> menuTemplate, FName menuLevel)
{
	m_mainMenuTemplate = menuTemplate;
	m_mainMenuLevel = menuLevel;
}

void UCoopGameInstance::ShowMainMenu_Implementation()
{
	if (IsCurrentState(CoopGameState::Playing))
		UGameplayStatics::OpenLevel(GetWorld(), m_mainMenuLevel);

	TransitionToState(CoopGameState::MainMenu);

	if (!IsValid(m_mainMenuWidget))
	{
		m_mainMenuWidget = CreateWidget<UUserWidget>(UGameplayStatics::GetPlayerController(GetWorld(), 0), m_mainMenuTemplate);
		m_mainMenuWidget->AddToViewport();

		FInputModeUIOnly inputModeUIOnly;
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetInputMode(inputModeUIOnly);
	}
}

void UCoopGameInstance::DebugError(const char* fmt, ...) const
{
	static const auto displayDuration = 3.0f;
	static const auto forceAlwaysDisplay = -1;

	if (GEngine != nullptr)
	{
		char buffer[256];

		va_list args;
		va_start(args, fmt);
		vsprintf(buffer, fmt, args);
		va_end(args);

		GEngine->AddOnScreenDebugMessage(forceAlwaysDisplay, displayDuration, FColor::Yellow, buffer);
	}
}
