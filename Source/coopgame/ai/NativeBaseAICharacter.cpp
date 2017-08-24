// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "NativeBaseAICharacter.h"
#include "items/NativeWeaponBase.h"

ANativeBaseAICharacter::ANativeBaseAICharacter(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	bUseControllerRotationYaw = true;
}

void ANativeBaseAICharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANativeBaseAICharacter, CurrentWeapon);
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
	UE_LOG(LogCoopGame, Log, TEXT("ANativeBaseAICharacter::StopWeaponFire() -> bWantsToFire = %s"), bWantsToFire ? TEXT("true") : TEXT("false"));
	if (bWantsToFire)
	{
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