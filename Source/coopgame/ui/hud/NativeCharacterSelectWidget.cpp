// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "NativeCharacterSelectWidget.h"
#include "NativeCoopPlayerController.h"

UNativeCharacterSelectWidget::UNativeCharacterSelectWidget(const FObjectInitializer& objectInitializer) : Super(objectInitializer)
{
	static ConstructorHelpers::FClassFinder<ANativeCoopCharacter> PlayerPawnOb(TEXT("/Game/Player/CoopCharacter2"));
	DefaultCharacter = PlayerPawnOb.Class;
}

void UNativeCharacterSelectWidget::SelectCharacter(int32 characterIndex)
{
	if (GetOwningPlayer() && Cast<ANativeCoopPlayerController>(GetOwningPlayer()))
	{
		auto playerController = Cast<ANativeCoopPlayerController>(GetOwningPlayer());
		playerController->SetPlayerCharacter(DefaultCharacter);
	}
}