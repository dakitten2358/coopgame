// Fill out your copyright notice in the Description page of Project Settings.

#include "NativeWeaponInstant.h"
#include "NativeBaseCharacter.h"
#include "NativeCoopCharacter.h"
#include "Kismet/GameplayStatics.h"

static const float WeaponRange = 15000.0f;
static const float AllowedViewDotHitDir = -1.0f;
static const float ClientSideHitLeeway = 200.0f;
static const float HitDamage = 34.0f;

void ANativeWeaponInstant::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ANativeWeaponInstant, HitImpactNotify, COND_SkipOwner);
}

void ANativeWeaponInstant::FireWeapon()
{
	auto aimDirection = GetAdjustedAim();
	auto shootOrigin = GetMuzzleLocation();

	auto shootDirection = aimDirection;

	// we'll use the camera for the actual trace origin, but switch to the muzzle location for hte effect
	const auto& startTrace = GetCameraDamageStartLocation(shootDirection);
	const auto& endTrace = startTrace + shootDirection * WeaponRange;	
	auto hitResult = WeaponTrace(startTrace, endTrace, FName(TEXT("InstantWeapon")));
	if (hitResult.bBlockingHit)
	{
		const FVector adjustedDirection = (hitResult.ImpactPoint - shootOrigin).GetSafeNormal();
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
			FHitResult MuzzleImpact = WeaponTrace(muzzleStartTrace, muzzleEndTrace, FName(TEXT("InstantWeapon")));

			if (MuzzleImpact.bBlockingHit)
			{
				bWeaponPenetration = true;
			}
		}

		if (bWeaponPenetration)
		{
			// spawn at crosshair position
			shootOrigin = hitResult.ImpactPoint - shootDirection * 10.0f;
		}
		else
		{
			// adjust direction to hit
			shootDirection = adjustedDirection;
		}
	}
	else
	{
		// didn't hit anything, so we'll use the max distance as the impact point
		hitResult.ImpactPoint = FVector_NetQuantize(endTrace);
	}

	ProcessInstantHit(hitResult, shootOrigin, shootDirection);
}

void ANativeWeaponInstant::ProcessInstantHit(const FHitResult& hitResult, const FVector& shootOrigin, const FVector& shootDirection)
{
	if (OwningCharacter && OwningCharacter->IsLocallyControlled() && GetNetMode() == NM_Client)
	{
		// if we are a client, and hit something that is controlled by the server
		if (hitResult.GetActor() && hitResult.GetActor()->GetRemoteRole() == ROLE_Authority)
		{
			// notify the server of our hit and let it validate and apply the damage
			ServerNotifyHit(hitResult, shootDirection);
		}
		else if (hitResult.GetActor() == nullptr)
		{
			if (hitResult.bBlockingHit)
				ServerNotifyHit(hitResult, shootDirection);
			else
				ServerNotifyMiss(shootDirection);
		}
	}

	// process a confirmed hit (??)
	ProcessInstantHitConfirmed(hitResult, shootOrigin, shootDirection);
}

bool ANativeWeaponInstant::ServerNotifyHit_Validate(const FHitResult hitResult, FVector_NetQuantizeNormal shootDirection)
{
	return true;
}


void ANativeWeaponInstant::ServerNotifyHit_Implementation(const FHitResult hitResult, FVector_NetQuantizeNormal shootDirection)
{
	// If we have an instigator, calculate the dot between the view and the shot
	if (Instigator && (hitResult.GetActor() || hitResult.bBlockingHit))
	{
		const FVector shootOrigin = GetMuzzleLocation();
		const FVector viewDirection = (hitResult.Location - shootOrigin).GetSafeNormal();

		const float viewDotHitDir = FVector::DotProduct(Instigator->GetViewRotation().Vector(), viewDirection);
		if (viewDotHitDir > AllowedViewDotHitDir)
		{
			// TODO: Check for weapon state

			if (hitResult.GetActor() == nullptr)
			{
				if (hitResult.bBlockingHit)
				{
					ProcessInstantHitConfirmed(hitResult, shootOrigin, shootDirection);
				}
			}
			// Assume it told the truth about static things because we don't move and the hit
			// usually doesn't have significant gameplay implications
			else if (hitResult.GetActor()->IsRootComponentStatic() || hitResult.GetActor()->IsRootComponentStationary())
			{
				ProcessInstantHitConfirmed(hitResult, shootOrigin, shootDirection);
			}
			else
			{
				const FBox hitBox = hitResult.GetActor()->GetComponentsBoundingBox();

				FVector boxExtent = 0.5 * (hitBox.Max - hitBox.Min);
				boxExtent *= ClientSideHitLeeway;

				boxExtent.X = FMath::Max(20.0f, boxExtent.X);
				boxExtent.Y = FMath::Max(20.0f, boxExtent.Y);
				boxExtent.Z = FMath::Max(20.0f, boxExtent.Z);

				const FVector boxCenter = (hitBox.Min + hitBox.Max) * 0.5;

				// If we are within client tolerance
				if (FMath::Abs(hitResult.Location.Z - boxCenter.Z) < boxExtent.Z &&
					FMath::Abs(hitResult.Location.X - boxCenter.X) < boxExtent.X &&
					FMath::Abs(hitResult.Location.Y - boxCenter.Y) < boxExtent.Y)
				{
					ProcessInstantHitConfirmed(hitResult, shootOrigin, shootDirection);
				}
			}
		}
	}

	// TODO: UE_LOG on failures & rejection
}

bool ANativeWeaponInstant::ServerNotifyMiss_Validate(FVector_NetQuantizeNormal shootDirection)
{
	return true;
}


void ANativeWeaponInstant::ServerNotifyMiss_Implementation(FVector_NetQuantizeNormal shootDirection)
{
	const FVector shootOrigin = GetMuzzleLocation();
	const FVector shootEnd = shootOrigin + (shootDirection * WeaponRange);

	// Play on remote clients
	HitImpactNotify = shootEnd;

	if (GetNetMode() != NM_DedicatedServer)
	{
		SpawnTrailEffects(shootEnd);
	}
}

void ANativeWeaponInstant::ProcessInstantHitConfirmed(const FHitResult& hitResult, const FVector& shootOrigin, const FVector& shootDirection)
{
	// Handle damage
	if (hitResult.GetActor() && ShouldDealDamage(hitResult.GetActor()))
	{
		DealDamage(hitResult, shootDirection);
	}

	// Play FX on remote clients
	if (Role == ROLE_Authority)
	{
		HitImpactNotify = hitResult.ImpactPoint;
	}

	// Play FX locally
	if (GetNetMode() != NM_DedicatedServer)
	{
		SimulateInstantHit(hitResult.ImpactPoint);
	}
}

bool ANativeWeaponInstant::ShouldDealDamage(AActor* toActor) const
{
	check(toActor);

	// If we are an actor on the server, or the local client has authoritative control over actor, we should register damage.
	if (GetNetMode() != NM_Client || toActor->Role == ROLE_Authority || toActor->GetTearOff())
		return true;

	return false;
}

void ANativeWeaponInstant::DealDamage(const FHitResult& hitResult, const FVector& shootDirection)
{
	float ActualHitDamage = HitDamage;

	// special damage location (headshot, etc)
	/*
	USDamageType* DmgType = Cast<USDamageType>(DamageType->GetDefaultObject());
	UPhysicalMaterial * PhysMat = Impact.PhysMaterial.Get();
	if (PhysMat && DmgType)
	{
		if (PhysMat->SurfaceType == SURFACE_ZOMBIEHEAD)
		{
			ActualHitDamage *= DmgType->GetHeadDamageModifier();
		}
		else if (PhysMat->SurfaceType == SURFACE_ZOMBIELIMB)
		{
			ActualHitDamage *= DmgType->GetLimbDamageModifier();
		}
	}
	*/
	auto asCoopCharacter = hitResult.GetActor() ? Cast<ANativeCoopCharacter>(hitResult.GetActor()) : nullptr;
	if (asCoopCharacter)
	{
		ActualHitDamage = 8.0f;
	}

	FPointDamageEvent pointDmg;
	pointDmg.DamageTypeClass = TSubclassOf<UDamageType>(UDamageType::StaticClass());
	pointDmg.HitInfo = hitResult;
	pointDmg.ShotDirection = shootDirection;
	pointDmg.Damage = ActualHitDamage;

	hitResult.GetActor()->TakeDamage(pointDmg.Damage, pointDmg, OwningCharacter->Controller, this);
}

void ANativeWeaponInstant::OnRep_HitLocation()
{
	// Played on all remote clients
	SimulateInstantHit(HitImpactNotify);
}

void ANativeWeaponInstant::SimulateInstantHit(const FVector& impactPoint)
{
	const FVector shootOrigin = GetMuzzleLocation();

	/* Adjust direction based on desired crosshair impact point and muzzle location */
	const FVector aimDirection = (impactPoint - shootOrigin).GetSafeNormal();

	const FVector shootEnd = shootOrigin + (aimDirection * WeaponRange);
	const FHitResult hitResult = WeaponTrace(shootOrigin, shootEnd, FName(TEXT("InstantWeapon")));

	if (hitResult.bBlockingHit)
	{
		//TODO:
		//SpawnImpactEffects(hitResult);
		SpawnTrailEffects(hitResult.ImpactPoint);
	}
	else
	{
		SpawnTrailEffects(shootEnd);
	}
}

void ANativeWeaponInstant::SpawnTrailEffects(const FVector& impactPoint)
{
	// check survival game for more complete
	auto shootOrigin = GetMuzzleLocation();
	auto shootDirection = impactPoint - shootOrigin;

	// Only spawn if a minimum distance is satisfied.
	/*
	if (ShootDir.Size() < MinimumProjectileSpawnDistance)
	{
		return;
	}
	*/

	//if (BulletsShotCount % TracerRoundInterval == 0)
	if (true)
	{
		if (TracerFX)
		{
			shootDirection.Normalize();
			UGameplayStatics::SpawnEmitterAtLocation(this, TracerFX, shootOrigin, shootDirection.Rotation());
			WeaponTrace(shootOrigin, impactPoint, FName("TracerRound"));
		}
	}
}