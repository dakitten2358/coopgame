// Fill out your copyright notice in the Description page of Project Settings.

#include "NativeLobbyPlayerController.h"
#include "ui/hud/NativeCharacterSelectWidget.h"
#include "online/CoopGamePlayerState.h"

// -----------------------------------------
// AActor
// -----------------------------------------
void ANativeLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		// Clean up the widgets that may have been left over
		RemoveAllWidgets();

		// If the character select widget doesn't exist, we
		// need to create it, and make it visible.
		if (CharacterSelectWidget)
		{
			m_characterSelectWidget = CreateWidget<UNativeCharacterSelectWidget>(this, CharacterSelectWidget);
			ShowCharacterSelect();
		}
	}
}

// -----------------------------------------
// Character selection
// -----------------------------------------
void ANativeLobbyPlayerController::ShowCharacterSelect()
{
	if (m_characterSelectWidget)
	{
		// add it to the viewport, make it visible
		m_characterSelectWidget->AddToViewport();
		m_characterSelectWidget->SetVisibility(ESlateVisibility::Visible);

		// make sure the player input goes to the UI
		FInputModeUIOnly inputModeUIOnly;
		SetInputMode(inputModeUIOnly);

		// and mouse stuff
		bShowMouseCursor = true;
		bEnableClickEvents = true;
		bEnableMouseOverEvents = true;
	}
}

void ANativeLobbyPlayerController::HideCharacterSelect()
{
	if (m_characterSelectWidget)
	{
		// hide it, and remove it from the viewport
		m_characterSelectWidget->SetVisibility(ESlateVisibility::Hidden);
		m_characterSelectWidget->RemoveFromViewport();

		// make sure the controls go back to the game itself
		FInputModeGameOnly inputModeGameOnly;
		SetInputMode(inputModeGameOnly);

		// hide the mouse
		bShowMouseCursor = false;
		bEnableClickEvents = false;
		bEnableMouseOverEvents = false;
	}
}

void ANativeLobbyPlayerController::RemoveAllWidgets()
{
	auto world = GetWorld();
	if (world && world->IsGameWorld())
	{
		if (auto viewportClient = world->GetGameViewport())
		{
			viewportClient->RemoveAllViewportWidgets();
		}
	}
}

// Online
// ------------------------------------------
void ANativeLobbyPlayerController::SetPlayerCharacter(const FName& characterToUse)
{
	// if we're not authority, send a request to the server
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerSetPlayerCharacter(characterToUse);
		return;
	}

	// we are authority at this point, make sure have a player state
	if (PlayerState && Cast<ACoopGamePlayerState>(PlayerState))
	{
		// update the player state
		auto playerState = Cast<ACoopGamePlayerState>(PlayerState);
		playerState->SelectedCharacterID = characterToUse;
	}
}

void ANativeLobbyPlayerController::ServerSetPlayerCharacter_Implementation(const FName& characterToUse)
{
	SetPlayerCharacter(characterToUse);
}

bool ANativeLobbyPlayerController::ServerSetPlayerCharacter_Validate(const FName& characterToUse)
{
	return true;
}