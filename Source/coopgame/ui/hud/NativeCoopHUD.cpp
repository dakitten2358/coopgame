// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "NativeCoopHUD.h"
#include "NativeCoopCharacter.h"
#include "NativeCoopPlayerController.h"
#include "online/NativeCoopGameState.h"
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

void ANativeCoopHUD::BeginPlay()
{
	AHUD::BeginPlay();

	// instructions timer
	GetWorld()->GetTimerManager().SetTimer(m_timerHandleInstructions, this, &self_t::StopShowingTip, 3.0f, false);
}

void ANativeCoopHUD::StopShowingTip()
{
	shouldDrawInstructionsTip = false;
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

	if (GetWorld()->GetGameState())
	{
		auto index = 0;
		auto gameState = GetWorld()->GetGameState();
		for (auto playerState : gameState->PlayerArray)
		{
			auto character = FindCharacterFor(playerState);
			if (character)
				DrawPlayerInfobox(index, playerState, character);

			index++;
		}

		DrawTimeElapsed();
	}

	if (shouldDrawInstructionsTip)
		DrawInstructionsTip();
}

void ANativeCoopHUD::DrawPlayerInfobox(int index, const APlayerState* playerState, const ANativeCoopCharacter* character) const
{
	FColor otherColor(110,124,131,255);
	FColor myColor(220, 124, 131, 255);

	float verticalOffset = 10.0f + (30.0f * index);

	FCanvasTextItem textItem(FVector2D::ZeroVector, FText::GetEmpty(), DefaultFont, IsMe(playerState) ? myColor : otherColor);
	textItem.Text = FText::FromString(playerState->PlayerName);
	Canvas->DrawItem(textItem, 10.0f, verticalOffset);

	textItem.Text = FText::AsNumber(character->GetCurrentHealth());
	Canvas->DrawItem(textItem, 200.0f, verticalOffset);
}

const ANativeCoopCharacter* ANativeCoopHUD::FindCharacterFor(const APlayerState* playerState) const
{
	TArray<AActor*> coopCharacterActors;
	UGameplayStatics::GetAllActorsOfClass(this, ANativeCoopCharacter::StaticClass(), coopCharacterActors);
	for (auto asActor : coopCharacterActors)
	{
		auto coopCharacter = Cast<ANativeCoopCharacter>(asActor);
		if (coopCharacter->PlayerState && coopCharacter->PlayerState->UniqueId == playerState->UniqueId)
			return coopCharacter;
	}

	return nullptr;
}

bool ANativeCoopHUD::IsMe(const APlayerState* playerState) const
{
	auto Identity = Online::GetIdentityInterface();
	if (!Identity.IsValid())
		return false;

	return *Identity->GetUniquePlayerId(0).Get() == *playerState->UniqueId;
}

void ANativeCoopHUD::DrawInstructionsTip()
{
	float centerX = Canvas->ClipX / 2;
	float centerY = Canvas->ClipY / 2;

	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f), 0.0f, centerY - 30, Canvas->ClipX, 60);

	auto tipText = TEXT("Press F1 for instructions");

	float w, h;
	Canvas->TextSize(DefaultFont, tipText, w, h);
	
	FColor textColor(255, 255, 255, 255);
	FCanvasTextItem textItem(FVector2D::ZeroVector, FText::FromString(tipText), DefaultFont, textColor);
	Canvas->DrawItem(textItem, centerX - (w/2.0f), centerY - (h/2.0f));
}

void ANativeCoopHUD::DrawTimeElapsed()
{
	float centerX = Canvas->ClipX / 2;

	auto gameState = Cast<ANativeCoopGameState>(GetWorld()->GetGameState());
	if (gameState == nullptr)
		return;

	auto matchState = gameState->GetMatchState();

	FString timeText = FString::FormatAsNumber(gameState->TimeElapsed);
	FString stateText = TEXT("Playing");
	if (matchState == MatchState::WaitingToStart || matchState == MatchState::WaitingPostMatch)
	{
		timeText = FString::FormatAsNumber(gameState->TimeRemaining);
		if (matchState == MatchState::WaitingToStart)
			stateText = TEXT("Waiting to Start");
		else
			stateText = TEXT("Post match");
	}

	FColor otherColor(110, 124, 131, 255);
	FCanvasTextItem textItem(FVector2D::ZeroVector, FText::GetEmpty(), DefaultFont, otherColor);

	// time
	float w, h;
	Canvas->TextSize(DefaultFont, timeText, w, h);
	textItem.Text = FText::FromString(timeText);
	Canvas->DrawItem(textItem, centerX - (w / 2), 90);

	// state
	Canvas->TextSize(DefaultFont, stateText, w, h);
	textItem.Text = FText::FromString(stateText);
	Canvas->DrawItem(textItem, centerX - (w / 2), 50);
}