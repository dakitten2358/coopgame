// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "CoopTypes.h"
#include "NativeCharacterSelectWidget.h"
#include "NativeCoopPlayerController.h"
#include "online/NativeLobbyPlayerController.h"

UNativeCharacterSelectWidget::UNativeCharacterSelectWidget(const FObjectInitializer& objectInitializer) : Super(objectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UDataTable> characterDataTableFinder(TEXT("DataTable'/Game/Data/CharacterDataTable.CharacterDataTable'"));
	m_characterTable = characterDataTableFinder.Object;
}

UNativeCharacterSelectWidget::~UNativeCharacterSelectWidget()
{
}

void UNativeCharacterSelectWidget::SelectCharacterByName(const FName& characterName)
{
	if (m_characterTable)
	{
		auto characterRow = m_characterTable->FindRow<FCharacterInfoRow>(characterName, TEXT("UNativeCharacterSelectWidget::SelectCharacterByName"));
		if (characterRow)
		{
			if (auto coopPlayerController = Cast<ANativeCoopPlayerController>(GetOwningPlayer()))
			{
				coopPlayerController->SetPlayerCharacter(characterName);
			}
			else if (auto lobbyPlayerController = Cast<ANativeLobbyPlayerController>(GetOwningPlayer()))
			{
				// set the player character on the server
				lobbyPlayerController->SetPlayerCharacter(characterName);
			}			
		}
		else
		{
			UE_LOG(LogCoopGame, Error, TEXT("UNativeCharacterSelectWidget::SelectCharacterByName(\"%s\"): FindRow failed."), *characterName.ToString());
		}

	}
	else
	{
		UE_LOG(LogCoopGame, Error, TEXT("UNativeCharacterSelectWidget::SelectCharacterByName(): Failed to find character table"));
	}
}

