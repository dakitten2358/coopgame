// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "NativeCoopGameMode_Menu.generated.h"

/**
 * 
 */
UCLASS()
class ANativeCoopGameMode_Menu : public AGameModeBase
{
	GENERATED_UCLASS_BODY()

public:
	virtual void RestartPlayer(class AController* newPlayer) override;

	virtual TSubclassOf<AGameSession> GetGameSessionClass() const override;
};