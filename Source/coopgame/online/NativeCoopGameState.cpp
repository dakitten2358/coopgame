// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "UnrealNetwork.h"
#include "NativeCoopGameState.h"

ANativeCoopGameState::ANativeCoopGameState(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	TimeRemaining = WarmupTime;
	TimeElapsed = 0;
}

void ANativeCoopGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// reset the time remaining, in case it was overridden in a blueprint
	TimeRemaining = WarmupTime;
}

void ANativeCoopGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// make sure the base stuff gets replicated
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// out stuff
	DOREPLIFETIME(ANativeCoopGameState, TimeRemaining);
	DOREPLIFETIME(ANativeCoopGameState, TimeElapsed);
}

