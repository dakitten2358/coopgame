// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NativeInGameMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class UNativeInGameMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHideMenuRequestedEvent);

	// called when the menu wants to close
	UPROPERTY(BlueprintAssignable, Category = "Widget Event")
	FOnHideMenuRequestedEvent OnHideMenuRequested;

	UFUNCTION(BlueprintCallable)
	void BroadcastHideMenuRequested();
};
