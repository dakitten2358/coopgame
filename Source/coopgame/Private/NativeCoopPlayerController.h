// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "NativeCoopCharacter.h"
#include "NativeCoopPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ANativeCoopPlayerController : public APlayerController
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

	UPROPERTY(EditDefaultsOnly, Category = "User Interface")
	TSubclassOf<class UNativePostMatchWidget> PostMatchWidget;

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

	UFUNCTION(BlueprintCallable)
	void ShowPostMatchWidget();

	// -----------------------------------------
	// Online
	// -----------------------------------------
	UFUNCTION(reliable, client)
	void ClientStartOnlineGame();

	UFUNCTION(reliable, client)
	void ClientEndOnlineGame();

	void SetPlayerCharacter(const FName& characterToUse);
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerSetPlayerCharacter(const FName& characterToUse);
	void ServerSetPlayerCharacter_Implementation(const FName& characterToUse);
	bool ServerSetPlayerCharacter_Validate(const FName& characterToUse);

	UFUNCTION(reliable, client)
	void ClientHandleMatchStarting();

	UFUNCTION(reliable, client)
	void ClientHandleMatchEnding();

	virtual void GameHasEnded(class AActor*, bool isWinner) override;

private:
	// retry timer handle while waiting for the player state to be replicated
	FTimerHandle m_timerHandleWaitingForPlayerState;

	// widgets
	class UNativeInstructionsWidget* m_instructionsWidget;
	class UNativeInGameMenuWidget* m_inGameMenuWidget;

	UPROPERTY()
	class UNativeCharacterSelectWidget* m_characterSelectWidget;

	UPROPERTY()
	class UNativePostMatchWidget* m_postMatchWidget;

	void RemoveAllWidgets();

	// convenience
	typedef ANativeCoopPlayerController self_t;	
};
