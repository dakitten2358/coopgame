// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/HUD.h"
#include "NativeCoopHUD.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ANativeCoopHUD : public AHUD
{
	GENERATED_BODY()

public:
	ANativeCoopHUD(const FObjectInitializer& objectInitializer);

	UPROPERTY(EditDefaultsOnly)
	UTexture2D* MainHUDTexture;

public:
	virtual void DrawHUD() override;
	
protected:
	FCanvasIcon m_crosshairCenterIcon;

	UPROPERTY()
	class UFont* DefaultFont;

private:
	void DrawPlayerInfobox(int index, const class APlayerState* playerState, const class ANativeCoopCharacter* character) const;
	const class ANativeCoopCharacter* FindCharacterFor(const class APlayerState* playerState) const;
	bool IsMe(const class APlayerState* playerState) const;
};
