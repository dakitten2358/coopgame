// Fill out your copyright notice in the Description page of Project Settings.

#include "NativeCoopGameInstance.h"
#include "coopgame.h"
#include "NativeBaseCharacter.h"
#include "NativeCoopPlayerController.h"
#include <Online.h>
#include <OnlineSubsystem.h>
#include <OnlineIdentityInterface.h>
#include <OnlineSessionInterface.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include <Runtime/UMG/Public/UMG.h>
#include "online/CoopGameSession.h"

UNativeCoopGameInstance::UNativeCoopGameInstance(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
	, m_isOnline(true)
{

}

// ------------------------------------------
// UGameInstance
// ------------------------------------------
void UNativeCoopGameInstance::Init()
{
	UE_LOG(LogCoopGame, Log, TEXT("UNativeCoopGameInstance::Init"));
	Super::Init();

	/*
	// general app delegate bindings
	FCoreDelegates::ApplicationWillDeactivateDelegate.AddUObject(this, &self_t::HandleApplicationWillDeactivate);
	FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddUObject(this, &self_t::HandleApplicationSuspend);
	FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddUObject(this, &self_t::HandleApplicationResume);
	FCoreDelegates::ApplicationLicenseChange.AddUObject(this, &self_t::HandleApplicationLicenseChange);
	
	FCoreDelegates::OnSafeFrameChangedEvent.AddUObject(this, &self_t::HandleSafeFrameChanged);
	FCoreDelegates::OnControllerConnectionChange.AddUObject(this, &self_t::HandleControllerConnectionChange);

	// map delegate bindings
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &self_t::OnPostLoadMapWithWorld);

	// bind ourselves to online subsystem callbacks we need
	const auto onlineSubsystem = IOnlineSubsystem::Get();
	check(onlineSubsystem);
	onlineSubsystem->AddOnConnectionStatusChangedDelegate_Handle(FOnConnectionStatusChangedDelegate::CreateUObject(this, &self_t::HandleNetworkConnectionStatusChanged));

	// identities
	const auto identityInterface = onlineSubsystem->GetIdentityInterface();
	check(identityInterface.IsValid());
	for(int i = 0; i < MAX_LOCAL_PLAYERS; ++i)
		identityInterface->AddOnLoginStatusChangedDelegate_Handle(i, FOnLoginStatusChangedDelegate::CreateUObject(this, &self_t::HandleUserLoginChanged));
	identityInterface->AddOnControllerPairingChangedDelegate_Handle(FOnControllerPairingChangedDelegate::CreateUObject(this, &self_t::HandleControllerPairingChanged));

	// session stuff
	const auto sessionInterface = onlineSubsystem->GetSessionInterface();
	check(sessionInterface.IsValid());
	sessionInterface->AddOnSessionFailureDelegate_Handle(FOnSessionFailureDelegate::CreateUObject(this, &self_t::HandleSessionFailure));

	// set up a timer for ticking
	m_tickDelegate = FTickerDelegate::CreateUObject(this, &self_t::Tick);
	m_tickDelegateHandle = FTicker::GetCoreTicker().AddTicker(m_tickDelegate);
	*/
}

void UNativeCoopGameInstance::Shutdown()
{
	Super::Shutdown();
}

bool UNativeCoopGameInstance::Tick(float deltaSeconds)
{
	// dedicated server doesn't care about any of this stuff
	if (IsRunningDedicatedServer())
		return true;

	return true;
}

void UNativeCoopGameInstance::StartGameInstance()
{
	// nothing to do here - defaults to start up
	Super::StartGameInstance();
}

void UNativeCoopGameInstance::HandleApplicationWillDeactivate()
{
}

void UNativeCoopGameInstance::HandleApplicationSuspend()
{
	// players will lose connection upon resume, it is possible however, that we'll never resume, so we need to end hte game here
	UE_LOG(LogCoopGameOnline, Warning, TEXT("UNativeCoopGameInstance::HandleApplicationSuspend"));
	UE_LOG(LogCoopGameNotImplemented, Error, TEXT("UNativeCoopGameInstance::HandleApplicationSuspend"));

}

void UNativeCoopGameInstance::HandleApplicationResume()
{
	UE_LOG(LogCoopGameOnline, Warning, TEXT("UNativeCoopGameInstance::HandleApplicationResume"));
}

void UNativeCoopGameInstance::HandleApplicationLicenseChange()
{
	auto app = FSlateApplication::Get().GetPlatformApplication();
	m_isLicensed = app->ApplicationLicenseValid();
}

void UNativeCoopGameInstance::HandleSafeFrameChanged()
{
	UCanvas::UpdateAllCanvasSafeZoneData(); //??
}

void UNativeCoopGameInstance::HandleNetworkConnectionStatusChanged(EOnlineServerConnectionStatus::Type previousConnectionStatus, EOnlineServerConnectionStatus::Type newConnectionStatus)
{
	UE_LOG(LogCoopGameOnline, Warning, TEXT("UNativeCoopGameInstance::HandleNetworkConnectionStatusChanged: %s"), EOnlineServerConnectionStatus::ToString(newConnectionStatus));
	UE_LOG(LogCoopGameTodo, Warning, TEXT("UI FOR CONSOLES"));
}

void UNativeCoopGameInstance::HandleSessionFailure(const FUniqueNetId& netId, ESessionFailure::Type failureType)
{
	UE_LOG(LogCoopGameOnline, Warning, TEXT("UNativeCoopGameInstance::HandleSessionFailure: failureType = %u"), (uint32) failureType);
	UE_LOG(LogCoopGameTodo, Warning, TEXT("ui for consoles"));
}

void UNativeCoopGameInstance::HandleControllerConnectionChange(bool isConnection, int32 unused, int32 gameUserIndex)
{
	UE_LOG(LogCoopGameOnline, Log, TEXT("UNativeCoopGameInstance::HandleControllerConnectionChange: isConnection = %s, gameUserIndex = %d"), isConnection ? TEXT("true") : TEXT("false"), gameUserIndex);

	if (!isConnection)
	{
		// controller was disconnected
		auto localPlayer = FindLocalPlayerFromControllerId(gameUserIndex);
		if (localPlayer == nullptr)
			return;

		// invalidate the controller id
		static const int invalidControllerId = -1;
		localPlayer->SetControllerId(invalidControllerId);
	}
}


void UNativeCoopGameInstance::HandleSignInChangeMessaging()
{
	
}


void UNativeCoopGameInstance::HandleUserLoginChanged(int32 gameUserIndex, ELoginStatus::Type previousLoginStatus, ELoginStatus::Type newLoginStatus, const FUniqueNetId& userID)
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
			HandleSignInChangeMessaging();
		}
		else
		{
			// remove local players from the list
			RemoveExistingLocalPlayer(localPlayer);
		}
	}
}

void UNativeCoopGameInstance::RemoveExistingLocalPlayer(ULocalPlayer* localPlayer)
{
	check(localPlayer);

	// controlling something?
	auto playerController = localPlayer->PlayerController;
	if (playerController != nullptr)
	{
		// if we have a pawn, let's kill it
		auto controlledPawn = Cast<ANativeBaseCharacter>(playerController->GetPawn());
		if (controlledPawn != nullptr)
			controlledPawn->KilledBy(nullptr);
	}

	// remove local split screen player from list
	RemoveLocalPlayer(localPlayer);
}

bool UNativeCoopGameInstance::IsLocalPlayerOnline(ULocalPlayer* localPlayer)
{
	if (localPlayer == nullptr)
		return false;

	// need an online subsystem to be online
	auto onlineSubsystem = IOnlineSubsystem::Get();
	if (onlineSubsystem == nullptr)
		return false;

	// need ot be able to identify userss to be onlline
	auto identityInterface = onlineSubsystem->GetIdentityInterface();
	if (!identityInterface.IsValid())
		return false;

	// need a valid id to be online
	auto uniqueId = localPlayer->GetCachedUniqueNetId();
	if (!uniqueId.IsValid())
		return false;

	auto loginStatus = identityInterface->GetLoginStatus(*uniqueId);
	return loginStatus == ELoginStatus::LoggedIn;
}

void UNativeCoopGameInstance::HandleControllerPairingChanged(int32 gameUserIndex, const FUniqueNetId& previousUser, const FUniqueNetId& newUser)
{
	UE_LOG(LogCoopGameOnline, Log, TEXT("HandleControllerPairingChanged: gameUserIndex = %d, previousUser = %s, newUser = %s"), gameUserIndex, *previousUser.ToString(), *newUser.ToString());

	// console : xbox one specific stuff
}

void UNativeCoopGameInstance::OnPostLoadMapWithWorld(class UWorld* world)
{
	UE_LOG(LogCoopGameTodo, Log, TEXT("Make sure loading screen is hidden..."));
}

// ------------------------------------------
// Online
// ------------------------------------------
bool UNativeCoopGameInstance::IsOnline() const
{
	return m_isOnline;
}

void UNativeCoopGameInstance::SetIsOnline(bool isOnline)
{
	m_isOnline = isOnline;

	// make sure we enable/disable for all the local players
	auto onlineSubsystem = IOnlineSubsystem::Get();
	if (onlineSubsystem)
	{
		foreach_localplayer([=](int index, ULocalPlayer* player) {
			auto userId = player->GetPreferredUniqueNetId();
			if (userId->IsValid())
				onlineSubsystem->SetUsingMultiplayerFeatures(*userId, m_isOnline);
		});
	}
}

void UNativeCoopGameInstance::SetMouseCursorEnabled(APlayerController* forController, bool enabled)
{
       forController->bShowMouseCursor = enabled;
       forController->bEnableClickEvents = enabled;
       forController->bEnableMouseOverEvents = enabled;
}

ACoopGameSession* UNativeCoopGameInstance::GetGameSession() const
{
	auto world = GetWorld();
	if (world)
	{
		auto game = world->GetAuthGameMode();
		if (game)
		{
			return Cast<ACoopGameSession>(game->GameSession);
		}
	}

	return nullptr;
}