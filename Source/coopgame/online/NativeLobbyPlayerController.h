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

	// -----------------------------------------
	// AActor
	// -----------------------------------------
public:
	virtual void BeginPlay() override;

	// -----------------------------------------
	// User Interface
	// -----------------------------------------
	// Character selection
	// -----------------------------------------
public:
	UPROPERTY(EditDefaultsOnly, Category = "User Interface")
	TSubclassOf<class UNativeCharacterSelectWidget> CharacterSelectWidget;

	UFUNCTION(BlueprintCallable, Category = "User Interface")
	void ShowCharacterSelect();

	UFUNCTION(BlueprintCallable, Category = "User Interface")
	void HideCharacterSelect();

	// Online
	// ------------------------------------------
	void SetPlayerCharacter(const FName& characterToUse);
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerSetPlayerCharacter(const FName& characterToUse);
	void ServerSetPlayerCharacter_Implementation(const FName& characterToUse);
	bool ServerSetPlayerCharacter_Validate(const FName& characterToUse);

private:
	UPROPERTY()
	class UNativeCharacterSelectWidget* m_characterSelectWidget = nullptr;

	void RemoveAllWidgets();
	// -----------------------------------------
};
