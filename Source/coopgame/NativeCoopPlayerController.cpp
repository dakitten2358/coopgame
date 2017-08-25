// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "NativeCoopPlayerController.h"
#include "CoopPlayerCameraManager.h"
#include "online/CoopGamePlayerState.h"
#include "OnlineSubsystem.h"
#include "Online.h"
#include "Runtime/UMG/Public/UMG.h"
#include "ui/hud/NativeInGameMenuWidget.h"
#include "ui/hud/NativeInstructionsWidget.h"
#include "ui/hud/NativeCharacterSelectWidget.h"
#include "GameFramework/GameMode.h"

ANativeCoopPlayerController::ANativeCoopPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, m_instructionsWidget(nullptr)
	, m_inGameMenuWidget(nullptr)
{
	// assign the class types we wish to use
	PlayerCameraManagerClass = ACoopPlayerCameraManager::StaticClass();
}

// -----------------------------------------
// AActor
// -----------------------------------------
void ANativeCoopPlayerController::BeginPlay()
{
	if (IsLocalPlayerController())
	{
		if (InstructionsWidget)
		{
			m_instructionsWidget = CreateWidget<UNativeInstructionsWidget>(this, InstructionsWidget);
			if (m_instructionsWidget)
			{
				m_instructionsWidget->AddToViewport();
				m_instructionsWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}

		if (InGameMenuWidget)
		{
			m_inGameMenuWidget = CreateWidget<UNativeInGameMenuWidget>(this, InGameMenuWidget);
			if (m_inGameMenuWidget)
			{
				m_inGameMenuWidget->AddToViewport();
				m_inGameMenuWidget->SetVisibility(ESlateVisibility::Hidden);
				m_inGameMenuWidget->OnHideMenuRequested.AddDynamic(this, &self_t::HideInGameMenu);
			}
		}

		if (CharacterSelectWidget)
		{
			m_characterSelectWidget = CreateWidget<UNativeCharacterSelectWidget>(this, CharacterSelectWidget);
			if (m_characterSelectWidget)
			{
				m_characterSelectWidget->AddToViewport();
				m_characterSelectWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}

	Super::BeginPlay();

	ShowCharacterSelect();
}

// -----------------------------------------
// APlayerController
// -----------------------------------------	
void ANativeCoopPlayerController::SetupInputComponent()
{
	// let the base class do it's thing
	Super::SetupInputComponent();

	// bind our ui commands
	InputComponent->BindAction("Instructions", IE_Pressed, this, &self_t::OnShowInstructionsPressed);
	InputComponent->BindAction("Instructions", IE_Released, this, &self_t::OnShowInstructionsReleased);
	InputComponent->BindAction("ToggleMenu", IE_Pressed, this, &self_t::ShowInGameMenu);
}

// -----------------------------------------
// User Interface
// -----------------------------------------
void ANativeCoopPlayerController::OnShowInstructionsPressed()
{
	if (m_instructionsWidget)
	{
		m_instructionsWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void ANativeCoopPlayerController::OnShowInstructionsReleased()
{
	if (m_instructionsWidget)
	{
		m_instructionsWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

bool ANativeCoopPlayerController::IsGameMenuVisible() const
{
	UE_LOG(LogCoopGameNotImplemented, Error, TEXT("ANativeCoopPlayerController::IsGameMenuVisible()"));
	return false;
}

void ANativeCoopPlayerController::ShowInGameMenu()
{
	if (m_inGameMenuWidget)
	{
		m_inGameMenuWidget->SetVisibility(ESlateVisibility::Visible);
		
		FInputModeUIOnly inputModeUIOnly;
		SetInputMode(inputModeUIOnly);

		bShowMouseCursor = true;
		bEnableClickEvents = true;
		bEnableMouseOverEvents = true;
	}
}

void ANativeCoopPlayerController::HideInGameMenu()
{
	if (m_inGameMenuWidget)
	{
		m_inGameMenuWidget->SetVisibility(ESlateVisibility::Hidden);

		FInputModeGameOnly inputModeGameOnly;
		SetInputMode(inputModeGameOnly);

		bShowMouseCursor = false;
		bEnableClickEvents = false;
		bEnableMouseOverEvents = false;
	}
}

void ANativeCoopPlayerController::ShowCharacterSelect()
{
	if (m_characterSelectWidget)
	{
		m_characterSelectWidget->SetVisibility(ESlateVisibility::Visible);

		FInputModeUIOnly inputModeUIOnly;
		SetInputMode(inputModeUIOnly);

		bShowMouseCursor = true;
		bEnableClickEvents = true;
		bEnableMouseOverEvents = true;
	}
}

void ANativeCoopPlayerController::HideCharacterSelect()
{
	if (m_characterSelectWidget)
	{
		m_characterSelectWidget->SetVisibility(ESlateVisibility::Hidden);

		FInputModeGameOnly inputModeGameOnly;
		SetInputMode(inputModeGameOnly);

		bShowMouseCursor = false;
		bEnableClickEvents = false;
		bEnableMouseOverEvents = false;
	}
}

// -----------------------------------------
// Online
// -----------------------------------------
void ANativeCoopPlayerController::ClientStartOnlineGame_Implementation()
{
	if (!IsPrimaryPlayer())
		return;

	auto playerState = Cast<ACoopGamePlayerState>(PlayerState);
	if (playerState)
	{
		auto onlineSubsystem = IOnlineSubsystem::Get();
		if (onlineSubsystem)
		{
			auto sessions = onlineSubsystem->GetSessionInterface();
			if (sessions.IsValid())
			{
				UE_LOG(LogCoopGameOnline, Log, TEXT("Starting sesion %s on client"), *playerState->SessionName.ToString());
				sessions->StartSession(playerState->SessionName);
			}
		}
	}
	else
	{
		// player state hasn't been replicated yet, retry
		GetWorld()->GetTimerManager().SetTimer(m_timerHandleWaitingForPlayerState, this, &self_t::ClientStartOnlineGame_Implementation, 0.1f, false);
	}
}

void ANativeCoopPlayerController::ClientEndOnlineGame_Implementation()
{
	if (!IsPrimaryPlayer())
		return;

	auto playerState = Cast<ACoopGamePlayerState>(PlayerState);
	if (playerState)
	{
		auto onlineSubsystem = IOnlineSubsystem::Get();
		if (onlineSubsystem)
		{
			auto sessions = onlineSubsystem->GetSessionInterface();
			if (sessions.IsValid())
			{
				UE_LOG(LogCoopGameOnline, Log, TEXT("Ending session %s on client"), *playerState->SessionName.ToString());
				sessions->EndSession(playerState->SessionName);
			}
		}
	}
}

void ANativeCoopPlayerController::SetPlayerCharacter(TSubclassOf<ANativeCoopCharacter> characterToUse)
{
	UE_LOG(LogCoopGameWeapon, Verbose, TEXT("ANativeCoopPlayerController::SetPlayerCharacter()"));
	if (Role < ROLE_Authority)
	{
		ServerSetPlayerCharacter(characterToUse);
		return;
	}

	if (PlayerState && Cast<ACoopGamePlayerState>(PlayerState))
	{
		auto playerState = Cast<ACoopGamePlayerState>(PlayerState);
		playerState->SelectedCharacter = characterToUse;

		

		// if the match is already in progress, let's restart the player so that he ends up with the selected character
		AGameMode* gameMode = Cast<AGameMode>(GetWorld()->GetAuthGameMode());
		if (gameMode && gameMode->GetMatchState() == MatchState::InProgress)
		{
			auto existingPawn = GetPawn();
			UnPossess();
			existingPawn->Destroy();
			gameMode->RestartPlayer(this);
		}
	}
}

void ANativeCoopPlayerController::ServerSetPlayerCharacter_Implementation(TSubclassOf<ANativeCoopCharacter> characterToUse)
{
	SetPlayerCharacter(characterToUse);
}

bool ANativeCoopPlayerController::ServerSetPlayerCharacter_Validate(TSubclassOf<ANativeCoopCharacter> characterToUse)
{
	return true;
}
