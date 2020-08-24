// Fill out your copyright notice in the Description page of Project Settings.

#include "NativeLobbyGameMode.h"
#include "NativeLobbyPlayerController.h"

ANativeLobbyGameMode::ANativeLobbyGameMode(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	PlayerControllerClass = ANativeLobbyPlayerController::StaticClass();

	bUseSeamlessTravel = true;
}

void ANativeLobbyGameMode::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	m_countDown = 12;

	// set up a simple 1s timer
	auto& timerManager = GetWorldTimerManager();
	timerManager.SetTimer(m_defaultTimerHandle, this, &self_t::OnDefaultTimer, GetWorldSettings()->GetEffectiveTimeDilation(), true);
}

void ANativeLobbyGameMode::OnDefaultTimer()
{
	m_countDown--;
	if (m_countDown == 0)
	{
		// for now, we'll simply move to the small map when 15s have expired
		GetWorld()->ServerTravel("TestSmallMap", false, false);
	}
}
