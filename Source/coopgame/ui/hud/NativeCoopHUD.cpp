// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "NativeCoopHUD.h"
#include <Online.h>
#include <OnlineIdentityInterface.h>

ANativeCoopHUD::ANativeCoopHUD(const FObjectInitializer& objectInitializer)
	: AHUD(objectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDCenterDotObj(TEXT("/Game/UI/HUD/hud_main"));
	m_crosshairCenterIcon = UCanvas::MakeIcon(HUDCenterDotObj.Object, 0, 0, 64, 64);

	// Fonts are not included in dedicated server builds.
	#if !UE_SERVER
	{
		static ConstructorHelpers::FObjectFinder<UFont> DefaultFontOb(TEXT("/Game/UI/Fonts/NeuropolX"));
		DefaultFont = DefaultFontOb.Object;
	}
	#endif //!UE_SERVER
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

	FColor DrawColor(110,124,131,255);
	FColor MyColor(220, 124, 131, 255);

	FCanvasTextItem TextItem( FVector2D::ZeroVector, FText::GetEmpty(), DefaultFont, DrawColor );
	FCanvasTextItem TextItemMe(FVector2D::ZeroVector, FText::GetEmpty(), DefaultFont, MyColor);
	//TextItem.EnableShadow( FLinearColor::Black );
	//TextItem.Text = FText::FromString( TEXT("TestText") );
	

	if (GetWorld()->GetGameState())
	{
		float verticalOffset = 10.0f;

		auto Identity = Online::GetIdentityInterface();
		if (Identity.IsValid())
		{
			auto x = Identity->GetPlayerNickname(0);
			
		}

		auto gameState = GetWorld()->GetGameState();
		for (auto playerState : gameState->PlayerArray)
		{
			if (*Identity->GetUniquePlayerId(0).Get() == *playerState->UniqueId)
			{
				TextItemMe.Text = FText::FromString(playerState->PlayerName);
				Canvas->DrawItem(TextItemMe, verticalOffset, 10.0f);
			}
			else
			{
				TextItem.Text = FText::FromString(playerState->PlayerName);
				Canvas->DrawItem(TextItem, verticalOffset, 10.0f);
			}
			verticalOffset += 30.0f;
		}		
	}
	else
	{
		TextItem.Text = FText::FromString(TEXT("Failed to get game state"));
		Canvas->DrawItem(TextItemMe, 10.0f, 10.0f);
	}
}
