// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerState.h"
#include "NativeCoopCharacter.h"
#include "CoopGamePlayerState.generated.h"


/**
 * 
 */
UCLASS()
class COOPGAME_API ACoopGamePlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACoopGamePlayerState(const FObjectInitializer& objectInitializer);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* receivingState) override;
	
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category = "Player")
	//TSubclassOf<ANativeCoopCharacter> SelectedCharacter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category = "Player")
	FName SelectedCharacterID;
};
