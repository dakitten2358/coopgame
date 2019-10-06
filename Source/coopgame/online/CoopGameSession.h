// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "CoopTypes.h"
#include "Online.h"
#include "CoopGameSession.generated.h"

UCLASS()
class ACoopGameSession : public AGameSession
{
	GENERATED_UCLASS_BODY()

	// -----------------------------------------
	// AGameSession
	// -----------------------------------------
public:
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;

protected:
	// delegate for starting a new session
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;

	void OnStartOnlineGameComplete(FName sessionName, bool wasSuccessful);

	// -----------------------------------------
	// presence stuff
	// -----------------------------------------
private:
	typedef ACoopGameSession self_t;
};
