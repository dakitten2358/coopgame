// Fill out your copyright notice in the Description page of Project Settings.

#include "NativeBaseAICharacter.h"
#include "coopgame.h"
#include "items/NativeWeaponBase.h"
#include "Runtime/AIModule/Classes/Perception/PawnSensingComponent.h"
#include "NativeCoopCharacter.h"
#include "NativeBaseAIController.h"

ANativeBaseAICharacter::ANativeBaseAICharacter(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	bUseControllerRotationYaw = true;

	PawnSenses = objectInitializer.CreateDefaultSubobject<UPawnSensingComponent>(this, TEXT("Senses"));

	// reasonable defaults
	PawnSenses->HearingThreshold = 600.0f;
	PawnSenses->LOSHearingThreshold = 2000.0f;
	PawnSenses->SightRadius = 2000.0f;
	PawnSenses->SetPeripheralVisionAngle(55.0f);
}

void ANativeBaseAICharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANativeBaseAICharacter, CurrentWeapon);
}

void ANativeBaseAICharacter::BeginPlay()
{
	Super::BeginPlay();

	if (PawnSenses)
	{
		PawnSenses->OnSeePawn.AddDynamic(this, &self_t::OnAISeePawn);
		PawnSenses->OnHearNoise.AddDynamic(this, &self_t::OnAIHearNoise);
	}
}

void ANativeBaseAICharacter::Destroyed()
{
	Super::Destroyed();

	// get rid of the current weapon if we have one (and we have authoritay)
	if (Role >= ROLE_Authority && CurrentWeapon != nullptr)
	{
		CurrentWeapon->Destroy();
		CurrentWeapon = nullptr;
	}
}


void ANativeBaseAICharacter::PossessedBy(AController* newController)
{
	Super::PossessedBy(newController);
	if (DefaultWeapon)
	{
		// add the weapon
		FActorSpawnParameters spawnInfo;
		spawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		auto newWeapon = GetWorld()->SpawnActor<ANativeWeaponBase>(DefaultWeapon, spawnInfo);

		// add it to the ai
		AddWeapon(newWeapon);
	}
}

// WEAPONS
// -----------------------------------------------------------------------------
void ANativeBaseAICharacter::AddWeapon(ANativeWeaponBase* weapon)
{
	// only the server can tell us to add a weapon
	if (weapon != nullptr && Role == ROLE_Authority)
	{
		UE_LOG(LogCoopGame, Log, TEXT("Equipping weapon"));
		weapon->OnEnterInventory(this);
		EquipWeapon(weapon);
	}
}

void ANativeBaseAICharacter::EquipWeapon(ANativeWeaponBase* weapon)
{
	if (weapon)
	{
		// Ignore if trying to equip already equipped weapon
		if (weapon == CurrentWeapon)
			return;

		if (Role == ROLE_Authority)
		{
			SetCurrentWeapon(weapon);
		}
		else
		{
			ServerEquipWeapon(weapon);
		}
	}
}

bool ANativeBaseAICharacter::ServerEquipWeapon_Validate(ANativeWeaponBase* Weapon)
{
	return true;
}

void ANativeBaseAICharacter::ServerEquipWeapon_Implementation(ANativeWeaponBase* Weapon)
{
	EquipWeapon(Weapon);
}

void ANativeBaseAICharacter::OnRep_CurrentWeapon(ANativeWeaponBase* oldWeapon)
{
	SetCurrentWeapon(CurrentWeapon);
}

void ANativeBaseAICharacter::SetCurrentWeapon(ANativeWeaponBase* weapon)
{
	UE_LOG(LogCoopGame, Log, TEXT("Setting weapon!"));
	CurrentWeapon = weapon;
	if (CurrentWeapon)
	{
		CurrentWeapon->SetOwningCharacter(this);

	}
}

ANativeWeaponBase* ANativeBaseAICharacter::GetCurrentWeapon() const
{
	return CurrentWeapon;
}

// FIRING
// -----------------------------------------------------------------------------
void ANativeBaseAICharacter::OnStartFire()
{
	// stop sprinting if we are
	if (IsSprinting())
		SetSprinting(false);

	// start firing the weapon
	StartWeaponFire();
}

void ANativeBaseAICharacter::OnStopFire()
{
	StopWeaponFire();
}

void ANativeBaseAICharacter::StartWeaponFire()
{
	UE_LOG(LogCoopGame, Log, TEXT("ANativeBaseAICharacter::StartWeaponFire() -> bWantsToFire = %s"), bWantsToFire ? TEXT("true") : TEXT("false"));
	if (!bWantsToFire)
	{
		bWantsToFire = true;
		if (CurrentWeapon)
		{
			CurrentWeapon->StartFire();
		}
		else
		{
			UE_LOG(LogCoopGame, Warning, TEXT("No weapon to fire!"));
		}
	}
}

void ANativeBaseAICharacter::StopWeaponFire()
{
	if (bWantsToFire)
	{
		UE_LOG(LogCoopGame, Log, TEXT("ANativeBaseAICharacter::StopWeaponFire() -> bWantsToFire = %s"), bWantsToFire ? TEXT("true") : TEXT("false"));
		bWantsToFire = false;
		if (CurrentWeapon)
		{
			CurrentWeapon->StopFire();
		}
		else
		{
			UE_LOG(LogCoopGame, Warning, TEXT("No weapon to stop fire!"));
		}
	}
}

bool ANativeBaseAICharacter::IsFiring() const
{
	if (CurrentWeapon && CurrentWeapon->GetCurrentState() == EWeaponState::Firing)
		return true;

	return false;
}

// SENSES
// -----------------------------------------------------------------------------
void ANativeBaseAICharacter::OnAISeePawn(APawn* Pawn)
{
	auto asPlayer = Cast<ANativeCoopCharacter>(Pawn);

	// if it's a player, let the controller know
	if (asPlayer != nullptr && GetController() && Cast<ANativeBaseAIController>(GetController()))
	{
		auto aiController = Cast<ANativeBaseAIController>(GetController());
		aiController->OnSawPlayer(asPlayer);
	}
}

void ANativeBaseAICharacter::OnAIHearNoise(APawn* NoiseInstigator, const FVector& Location, float Volume)
{
	auto asPlayer = Cast<ANativeCoopCharacter>(NoiseInstigator);
	
	// if it's a player, let the controller
	if (asPlayer != nullptr && GetController() && Cast<ANativeBaseAIController>(GetController()))
	{
		auto aiController = Cast<ANativeBaseAIController>(GetController());
		aiController->OnHeardPlayer(asPlayer);
	}
}

float ANativeBaseAICharacter::TakeDamage(float DamageAmount, const struct FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	auto f = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (EventInstigator && EventInstigator->GetPawn() && Cast<ANativeCoopCharacter>(EventInstigator->GetPawn()))
	{
		auto asPlayer = Cast<ANativeCoopCharacter>(EventInstigator->GetPawn());
		// if it's a player, let the controller
		if (asPlayer != nullptr && GetController() && Cast<ANativeBaseAIController>(GetController()))
		{
			auto aiController = Cast<ANativeBaseAIController>(GetController());
			aiController->OnTookDamageFromPlayer(asPlayer, DamageAmount);
		}
	}

	return f;
}
