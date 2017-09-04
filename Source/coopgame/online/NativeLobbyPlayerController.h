// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NativeLobbyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ANativeLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
	
public:
	virtual void BeginPlay() override;

private:
	void RemoveAllWidgets();
	
};
