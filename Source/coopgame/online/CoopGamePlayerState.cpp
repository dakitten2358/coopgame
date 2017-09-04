// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "CoopGamePlayerState.h"
#include "UnrealNetwork.h"

ACoopGamePlayerState::ACoopGamePlayerState(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{

}

void ACoopGamePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(ACoopGamePlayerState, SelectedCharacter);
	DOREPLIFETIME(ACoopGamePlayerState, SelectedCharacterID);
}

void ACoopGamePlayerState::CopyProperties(APlayerState* receivingState)
{
	Super::CopyProperties(receivingState);

	if (auto receivingCoopState = Cast<ACoopGamePlayerState>(receivingState))
	{
		receivingCoopState->SelectedCharacterID = SelectedCharacterID;
	}
}

