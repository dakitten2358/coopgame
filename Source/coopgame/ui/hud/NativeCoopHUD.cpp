// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "NativeCoopHUD.h"

ANativeCoopHUD::ANativeCoopHUD(const FObjectInitializer& objectInitializer)
	: AHUD(objectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDCenterDotObj(TEXT("/Game/UI/HUD/hud_main"));
	m_crosshairCenterIcon = UCanvas::MakeIcon(HUDCenterDotObj.Object, 0, 0, 64, 64);
}

void ANativeCoopHUD::DrawHUD()
{
	Super::DrawHUD();

	float centerX = Canvas->ClipX / 2;
	float centerY = Canvas->ClipY / 2;
	float crosshairCenterScale = 0.07f;

	Canvas->SetDrawColor(255, 255, 255, 255);
	Canvas->DrawIcon(
		m_crosshairCenterIcon,
		centerX - m_crosshairCenterIcon.UL * crosshairCenterScale / 2.0f,
		centerY - m_crosshairCenterIcon.VL * crosshairCenterScale / 2.0f,
		crosshairCenterScale);
}
