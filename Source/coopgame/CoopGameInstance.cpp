// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "CoopGameInstance.h"
#include <OnlineSubsystem.h>
#include <OnlineIdentityInterface.h>
#include <OnlineSessionInterface.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include <Runtime/UMG/Public/UMG.h>

UCoopGameInstance::UCoopGameInstance(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
	, m_isOnline(true)
{

}

// ------------------------------------------
// UGameInstance
// ------------------------------------------
void UCoopGameInstance::Init()
{
	Super::Init();

	// need the ability to ID users
	const auto onlineSubsystem = IOnlineSubsystem::Get();
	check(onlineSubsystem);

	const auto identityInterface = onlineSubsystem->GetIdentityInterface();
	check(identityInterface.IsValid());

	const auto sessionInterface = onlineSubsystem->GetSessionInterface();
	check(sessionInterface.IsValid());

	// bind ourselves to online subsystem callbacks we need
	for(int i = 0; i < MAX_LOCAL_PLAYERS; ++i)
		identityInterface->AddOnLoginStatusChangedDelegate_Handle(i, FOnLoginStatusChangedDelegate::CreateUObject(this, &UCoopGameInstance::HandleUserLoginChanged));
}

void UCoopGameInstance::Shutdown()
{
	Super::Shutdown();
}

void UCoopGameInstance::StartGameInstance()
{
	// nothing to do here - defaults to start up
}

void UCoopGameInstance::HandleUserLoginChanged(int32 gameUserIndex, ELoginStatus::Type previousLoginStatus, ELoginStatus::Type newLoginStatus, const FUniqueNetId& userID)
{
	bool wasDowngraded = (newLoginStatus == ELoginStatus::NotLoggedIn && !IsOnline()) ||
						 (newLoginStatus != ELoginStatus::LoggedIn && IsOnline());
	UE_LOG(LogCoopGameOnline, Log, TEXT("HandleUserLoginChanged: wasDowngraded = %s"), wasDowngraded ? TEXT("true") : TEXT("false"));

	// make sure we're licensed
	auto genericApplication = FSlateApplication::Get().GetPlatformApplication();
	m_isLicensed = genericApplication->ApplicationLicenseValid();

	// get the local player from the net id
	auto localPlayer = FindLocalPlayerFromUniqueNetId(userID);

	// if user is signed out, but was previously signed in, move him to the welcome screen
	if (localPlayer != nullptr && wasDowngraded)
	{
		UE_LOG(LogCoopGameOnline, Log, TEXT("HandleUserLoginChanged: Player logged out: %s"), *userID.ToString());

		//LabelPlayerAsQuitter(localPlayer);

		if (localPlayer == GetFirstGamePlayer() || IsOnline())
		{
			//HandleSignInChangeMessaging();
		}
		else
		{
			// remove local split screen players from the list
			//RemoveExistingLocalPlayer(localPlayer);
		}
	}
}

// ------------------------------------------
// Game State
// ------------------------------------------
bool UCoopGameInstance::IsCurrentState(CoopGameState inState) const
{
	return m_currentGameState == inState;
}

CoopGameState UCoopGameInstance::TransitionToState(CoopGameState newState)
{
	// tried to transition to the same state
	if (m_currentGameState == newState)
	{
		UE_LOG(LogCoopGame, Error, TEXT("Attempted to transition to the same state (%d -> %d)"), (int)m_currentGameState, (int)newState);
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

// ------------------------------------------
// Online
// ------------------------------------------
bool UCoopGameInstance::IsOnline() const
{
	return m_isOnline;
}
