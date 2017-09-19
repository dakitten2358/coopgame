// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "NativeCoopHUD.h"
#include "NativeCoopCharacter.h"
#include "NativeCoopPlayerController.h"
#include "online/NativeCoopGameState.h"
#include <Online.h>
#include <OnlineIdentityInterface.h>
#include "online/CoopGamePlayerState.h"
#include "CoopTypes.h"
#include <limits>

ANativeCoopHUD::ANativeCoopHUD(const FObjectInitializer& objectInitializer)
	: AHUD(objectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDCenterDotObj(TEXT("/Game/UI/HUD/hud_main"));
	m_crosshairCenterIcon = UCanvas::MakeIcon(HUDCenterDotObj.Object, 0, 0, 64, 64);

	// going to need access to the character table
	static ConstructorHelpers::FObjectFinder<UDataTable> characterDataTableFinder(TEXT("DataTable'/Game/Data/CharacterDataTable.CharacterDataTable'"));
	m_characterTable = characterDataTableFinder.Object;

	// Fonts are not included in dedicated server builds.
	#if !UE_SERVER
	{
		static ConstructorHelpers::FObjectFinder<UFont> DefaultFontOb(TEXT("/Game/UI/Fonts/NeuropolX"));
		DefaultFont = DefaultFontOb.Object;
	}
	#endif //!UE_SERVER

	m_hitIndicatorOffset = FVector2D(8, 8);
	m_hitIndicatorSize = FVector2D(16, 128);
}

void ANativeCoopHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (MainHUDTexture)
	{
		m_unitFrameBackground = UCanvas::MakeIcon(MainHUDTexture, 14, 385, 298, 123);
		m_unitFrameHealthbar = UCanvas::MakeIcon(MainHUDTexture, 98, 354, 210, 25);
	}
	
}

void ANativeCoopHUD::BeginPlay()
{
	AHUD::BeginPlay();

	// instructions timer
	GetWorld()->GetTimerManager().SetTimer(m_timerHandleInstructions, this, &self_t::StopShowingTip, 3.0f, false);
}

void ANativeCoopHUD::Tick(float elapsedTime)
{
	Super::Tick(elapsedTime);

	UpdateHitIndicatorTimers(elapsedTime);
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

#if 0
	static float rot = 0.0f;
	rot += 1.0f;

	FVector front(1, 0, 0);
	FVector left(0, -1, 0);

	const FVector HitVector = FRotationMatrix(PlayerOwner->GetControlRotation()).InverseTransformVector(left);
	auto cosTheta = FVector::DotProduct(front, HitVector);
	auto angle = FGenericPlatformMath::Acos(cosTheta);
	rot = FMath::RadiansToDegrees(angle);

	Canvas->K2_DrawTexture(
		HitIndicatorTexture,
		FVector2D(centerX - 16, centerY - 128),
		FVector2D(32, 128),
		FVector2D(0, 0),
		FVector2D::UnitVector,
		FLinearColor::White,
		BLEND_Translucent,
		rot,
		FVector2D(0.5F, 1.0F)
	);
#endif

	DrawHitIndicators(centerX, centerY);
	
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

void ANativeCoopHUD::DrawPlayerInfobox(int index, const APlayerState* playerState, const ANativeCoopCharacter* character)
{
	FColor otherColor(110,124,131,255);
	FColor myColor(220, 124, 131, 255);

	//textItem.Text = FText::AsNumber(character->GetCurrentHealth());
	//Canvas->DrawItem(textItem, 200.0f, verticalOffset);

	float margin = 30.0f;

	float horizontalOffset = margin + ((m_unitFrameBackground.UL + margin) * index);
	float verticalOffset = Canvas->ClipY - (margin + m_unitFrameBackground.VL);

	Canvas->DrawIcon(m_unitFrameBackground, horizontalOffset, verticalOffset);

	// text offset
	float textHorizontalOffset = 25.0f;
	float textVerticalOffset = 88.0f;

	FCanvasTextItem textItem(FVector2D::ZeroVector, FText::GetEmpty(), DefaultFont, IsMe(playerState) ? myColor : otherColor);
	textItem.Text = FText::FromString(playerState->PlayerName);
	Canvas->DrawItem(textItem, horizontalOffset + textHorizontalOffset, verticalOffset + textVerticalOffset);

	// health offset
	float healthHorizontalOffset = 84.0f;
	float healthVerticalOffset = 54.0f;

	const float HealthAmount = FMath::Min(1.0f, (float)character->GetCurrentHealth() / (float)character->GetMaxHealth());

	FCanvasTileItem TileItem(FVector2D(healthHorizontalOffset + horizontalOffset, healthVerticalOffset + verticalOffset), m_unitFrameHealthbar.Texture->Resource,
		FVector2D(m_unitFrameHealthbar.UL * HealthAmount, m_unitFrameHealthbar.VL ), FLinearColor::Green);
	MakeUV(m_unitFrameHealthbar, TileItem.UV0, TileItem.UV1, m_unitFrameHealthbar.U, m_unitFrameHealthbar.V, m_unitFrameHealthbar.UL * HealthAmount, m_unitFrameHealthbar.VL);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TileItem);


	//Canvas->DrawIcon(m_unitFrameHealthbar, healthHorizontalOffset + horizontalOffset, healthVerticalOffset + verticalOffset);

	auto coopPlayerState = Cast<ACoopGamePlayerState>(playerState);
	if (coopPlayerState && m_characterTable)
	{
		const auto& characterName = coopPlayerState->SelectedCharacterID;
		auto characterRow = m_characterTable->FindRow<FCharacterInfoRow>(characterName, TEXT("UNativeCharacterSelectWidget::SelectCharacterByName"));
		if (characterRow)
		{
			auto avatarTexture = characterRow->Icon;
			if (avatarTexture)
			{
				Canvas->K2_DrawTexture(const_cast<UTexture2D*>(avatarTexture),
					FVector2D(4.0f + horizontalOffset, 4.0f + verticalOffset),
					FVector2D(75.0f, 75.0f),
					FVector2D::ZeroVector,
					FVector2D::UnitVector);
			}
		}

	}
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

void ANativeCoopHUD::MakeUV(FCanvasIcon& Icon, FVector2D& UV0, FVector2D& UV1, uint16 U, uint16 V, uint16 UL, uint16 VL)
{
	if (Icon.Texture)
	{
		const float Width = Icon.Texture->GetSurfaceWidth();
		const float Height = Icon.Texture->GetSurfaceHeight();
		UV0 = FVector2D(U / Width, V / Height);
		UV1 = UV0 + FVector2D(UL / Width, VL / Height);
	}
}

void ANativeCoopHUD::NotifyWeaponHit(float DamageTaken, const struct FDamageEvent& DamageEvent, class APawn* PawnInstigator)
{
	// todo:
	FVector ImpulseDir;
	FHitResult Hit;
	DamageEvent.GetBestHitInfo(this, PawnInstigator, Hit, ImpulseDir);

	// find a free slot, or the oldest slot if available
	auto& hitIndicatorSlot = FindFreeHitIndicatorSlot();
	hitIndicatorSlot.HitImpulse = ImpulseDir;
	hitIndicatorSlot.TimeRemaining = HitIndicatorDuration;
}

ANativeCoopHUD::FHitByEnemyInfo& ANativeCoopHUD::FindFreeHitIndicatorSlot()
{
	auto lowestTimeRemaining = std::numeric_limits<float>::min();
	FHitByEnemyInfo* lowestInfoSoFar = nullptr;

	for (auto& currentInfo : EnemyHitIndicatorInfo)
	{
		// if we find one that's completely available, use it
		if (currentInfo.TimeRemaining <= 0.0f)
			return currentInfo;

		// keep track of the oldest (nearest to zero)
		if (lowestTimeRemaining < currentInfo.TimeRemaining)
		{
			lowestInfoSoFar = &currentInfo;
			lowestTimeRemaining = currentInfo.TimeRemaining;
		}
	}

	check(lowestInfoSoFar);
	return *lowestInfoSoFar;
}

void ANativeCoopHUD::DrawHitIndicators(float centerX, float centerY) const
{
	for (const auto& currentInfo : EnemyHitIndicatorInfo)
	{
		if (currentInfo.TimeRemaining <= 0.0f)
			continue;

		// figure out the angle we should be drawing it at
		const auto& impulseDirection = currentInfo.HitImpulse;
		const FVector hitVector = FRotationMatrix(PlayerOwner->GetControlRotation()).InverseTransformVector(-impulseDirection);

		auto angleInRadians = FGenericPlatformMath::Atan2(hitVector.Y, hitVector.X);
		auto angleInDegrees = FMath::RadiansToDegrees(angleInRadians);

		Canvas->K2_DrawTexture(
			MainHUDTexture,
			FVector2D(centerX - (m_hitIndicatorSize.X / 2), centerY - (m_hitIndicatorSize.Y)),
			FVector2D(m_hitIndicatorSize.X, m_hitIndicatorSize.Y),
			FVector2D(m_hitIndicatorOffset.X / 512.0f, m_hitIndicatorOffset.Y / 512.0f),
			FVector2D(m_hitIndicatorSize.X / 512.0f, m_hitIndicatorSize.Y / 512.0f),
			FLinearColor::Red,
			BLEND_Translucent,
			angleInDegrees,
			FVector2D(0.5f, 1.0f)
		);
	}
}

void ANativeCoopHUD::UpdateHitIndicatorTimers(float elapsedTime)
{
	for (auto& currentInfo : EnemyHitIndicatorInfo)
	{
		if (currentInfo.TimeRemaining <= 0.0f)
			continue;

		currentInfo.TimeRemaining -= elapsedTime;
	}
}
