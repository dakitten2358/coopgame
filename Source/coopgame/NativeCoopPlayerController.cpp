// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "NativeCoopPlayerController.h"
#include "CoopPlayerCameraManager.h"


ANativeCoopPlayerController::ANativeCoopPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// assign the class types we wish to use
	PlayerCameraManagerClass = ACoopPlayerCameraManager::StaticClass();
}

bool ANativeCoopPlayerController::IsGameMenuVisible() const
{
	UE_LOG(LogCoopGameNotImplemented, Error, TEXT("ANativeCoopPlayerController::IsGameMenuVisible()"));
	return false;
}

void ANativeCoopPlayerController::ShowInGameMenu()
{
	UE_LOG(LogCoopGameNotImplemented, Error, TEXT("ANativeCoopPlayerController::ShowInGameMenu"));
}