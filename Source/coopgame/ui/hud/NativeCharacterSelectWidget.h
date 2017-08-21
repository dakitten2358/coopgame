// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NativeCharacterSelectWidget.generated.h"

class ANativeCoopCharacter;
/**
 * 
 */
UCLASS()
class COOPGAME_API UNativeCharacterSelectWidget : public UUserWidget
{
	GENERATED_UCLASS_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SelectCharacter(int32 characterIndex);

private:
	TSubclassOf<ANativeCoopCharacter> DefaultCharacter;
	
	
};
