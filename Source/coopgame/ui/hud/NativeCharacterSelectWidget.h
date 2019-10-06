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
class UNativeCharacterSelectWidget : public UUserWidget
{
	GENERATED_UCLASS_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SelectCharacterByName(const FName& characterName);

	virtual ~UNativeCharacterSelectWidget();

private:
	const class UDataTable* m_characterTable;
};
