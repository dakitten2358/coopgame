// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "NativeLobbyPlayerController.h"

void ANativeLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	RemoveAllWidgets();
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
