// Fill out your copyright notice in the Description page of Project Settings.

#include "NativeCoopGameMode_Menu.h"
#include "online/CoopGameSession.h"




ANativeCoopGameMode_Menu::ANativeCoopGameMode_Menu(const FObjectInitializer& objectInitializer)
{
	// set up the player controller?
}

void ANativeCoopGameMode_Menu::RestartPlayer(class AController* newPlayer)
{
	// don't restart
}

TSubclassOf<AGameSession> ANativeCoopGameMode_Menu::GetGameSessionClass() const
{
	return Super::GetGameSessionClass();
}