// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "NativeCoopCharacter.h"
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

	// -----------------------------------------
	// AActor
	// -----------------------------------------
	virtual void BeginPlay() override;

	// -----------------------------------------
	// APlayerController
	// -----------------------------------------
	virtual void SetupInputComponent() override;

	// -----------------------------------------
	// User Interface
	// -----------------------------------------
	UPROPERTY(EditDefaultsOnly, Category = "User Interface")
	TSubclassOf<class UNativeInstructionsWidget> InstructionsWidget;

	UPROPERTY(EditDefaultsOnly, Category = "User Interface")
	TSubclassOf<class UNativeInGameMenuWidget> InGameMenuWidget;

	UPROPERTY(EditDefaultsOnly, Category = "User Interface")
	TSubclassOf<class UNativeCharacterSelectWidget> CharacterSelectWidget;

	bool IsGameMenuVisible() const;
	void ShowInGameMenu();
	UFUNCTION(BlueprintCallable)
	void HideInGameMenu();
	void OnShowInstructionsPressed();
	void OnShowInstructionsReleased();

	UFUNCTION(BlueprintCallable)
	void ShowCharacterSelect();
	UFUNCTION(BlueprintCallable)
	void HideCharacterSelect();

	// -----------------------------------------
	// Online
	// -----------------------------------------
	UFUNCTION(reliable, client)
	void ClientStartOnlineGame();

	UFUNCTION(reliable, client)
	void ClientEndOnlineGame();

	void SetPlayerCharacter(TSubclassOf<ANativeCoopCharacter> characterToUse);
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerSetPlayerCharacter(TSubclassOf<ANativeCoopCharacter> characterToUse);
	void ServerSetPlayerCharacter_Implementation(TSubclassOf<ANativeCoopCharacter> characterToUse);
	bool ServerSetPlayerCharacter_Validate(TSubclassOf<ANativeCoopCharacter> characterToUse);

	UFUNCTION(reliable, client)
	void ClientHandleMatchStarting();

private:
	// retry timer handle while waiting for the player state to be replicated
	FTimerHandle m_timerHandleWaitingForPlayerState;

	// widgets
	class UNativeInstructionsWidget* m_instructionsWidget;
	class UNativeInGameMenuWidget* m_inGameMenuWidget;

	UPROPERTY()
	class UNativeCharacterSelectWidget* m_characterSelectWidget;

	// convenience
	typedef ANativeCoopPlayerController self_t;	
};
