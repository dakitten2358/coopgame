// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "NativeCoopGameState.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ANativeCoopGameState : public AGameState
{
	GENERATED_UCLASS_BODY()
	
public:
	// time left for warmup / post match
	UPROPERTY(BlueprintReadOnly, Transient, Replicated)
	int32 TimeRemaining;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	int32 WarmupTime = 4 * 60;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	int32 PostMatchTime = 2 * 60;

	// time that has elapsed
	UPROPERTY(BlueprintReadOnly, Transient, Replicated)
	int32 TimeElapsed;

	// overrides
	virtual void PostInitializeComponents() override;
};
