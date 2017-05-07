// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "NativeCoopPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ANativeCoopPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ANativeCoopPlayerController(const FObjectInitializer& ObjectInitializer);

	bool IsGameMenuVisible() const;
	void ShowInGameMenu();

	
};
