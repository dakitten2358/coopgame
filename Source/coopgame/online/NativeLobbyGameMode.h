// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "NativeLobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ANativeLobbyGameMode : public AGameMode
{
	GENERATED_UCLASS_BODY()

public:
	virtual void PostInitializeComponents() override;	
	
protected:
	FTimerHandle m_defaultTimerHandle;
	void OnDefaultTimer();

private:
	typedef ANativeLobbyGameMode self_t;

	int m_countDown;
	
};
