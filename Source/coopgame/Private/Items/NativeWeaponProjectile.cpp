// Fill out your copyright notice in the Description page of Project Settings.

#include "NativeWeaponProjectile.h"
#include "NativeCoopCharacter.h"
#include "Kismet/GameplayStatics.h"

void ANativeWeaponProjectile::FireWeapon()
{
	auto aimDirection = GetAdjustedAim();
	auto shootOrigin = GetMuzzleLocation();

	auto accuracy = 1.0f;
	if (OwningCharacter && OwningCharacter->IsAimingDownSights())
		accuracy = 0.2f;

	const int32 randomSeed = FMath::Rand();
	FRandomStream weaponRandomStream(randomSeed);
	const float currentSpread = 10.0f * accuracy;
	const float coneHalfAngle = FMath::DegreesToRadians(currentSpread * 0.5f);
	auto shootDirection = weaponRandomStream.VRandCone(aimDirection, coneHalfAngle, coneHalfAngle);

	// trace from camera to check what's under crosshair
	const float projectilMaxRange = 10000.0f;
	const auto& startTrace = GetCameraDamageStartLocation(shootDirection);
	const auto& endTrace = startTrace + shootDirection * projectilMaxRange;
	FHitResult Impact = WeaponTrace(startTrace, endTrace, FName(TEXT("ProjectileWeapon")));

	// and adjust directions to hit that actor
	if (Impact.bBlockingHit)
	{
		const FVector adjustedDirection = (Impact.ImpactPoint - shootOrigin).GetSafeNormal();
		bool bWeaponPenetration = false;

		const float directionDot = FVector::DotProduct(adjustedDirection, shootDirection);
		if (directionDot < 0.0f)
		{
			// shooting backwards = weapon is penetrating
			bWeaponPenetration = true;
		}
		else if (directionDot < 0.5f)
		{
			// check for weapon penetration if angle difference is big enough
			// raycast along weapon mesh to check if there's blocking hit

			FVector muzzleStartTrace = shootOrigin - GetMuzzleDirection() * 150.0f;
			FVector muzzleEndTrace = shootOrigin;
			FHitResult MuzzleImpact = WeaponTrace(muzzleStartTrace, muzzleEndTrace, FName(TEXT("ProjectileWeapon")));

			if (MuzzleImpact.bBlockingHit)
			{
				bWeaponPenetration = true;
			}
		}

		if (bWeaponPenetration)
		{
			// spawn at crosshair position
			shootOrigin = Impact.ImpactPoint - shootDirection * 10.0f;
		}
		else
		{
			// adjust direction to hit
			shootDirection = adjustedDirection;
		}
	}

	ServerFireProjectile(shootOrigin, shootDirection);
}

bool ANativeWeaponProjectile::ServerFireProjectile_Validate(FVector shootOrigin, FVector_NetQuantizeNormal shootDirection)
{
	return true;
}

void ANativeWeaponProjectile::ServerFireProjectile_Implementation(FVector shootOrigin, FVector_NetQuantizeNormal shootDirection)
{
	FTransform spawnTransform(shootDirection.Rotation(), shootOrigin);

	if (DefaultProjectile != nullptr)
	{
		auto newProjectile = Cast<AActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, DefaultProjectile, spawnTransform));
		if (newProjectile)
		{
			newProjectile->SetInstigator(GetInstigator());
			newProjectile->SetOwner(this);
			//newProjectile->InitVelociy(shootDirection);

			UGameplayStatics::FinishSpawningActor(newProjectile, spawnTransform);
		}
	}
}
