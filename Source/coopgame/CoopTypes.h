// Fill out your copyright notice in the Description page of Project Settings.

#include "CoopTypes.generated.h"
#pragma once

UENUM(BlueprintType)
enum class CoopGameType : uint8
{
	Adventure		UMETA(DisplayName = "Adventure"),
	LastStand		UMETA(DisplayName = "Last Stand"),
};

UENUM()
enum class EHitResult
{
	Wounded,
	Dead,
};

USTRUCT()
struct FTakeHitInfo
{
	GENERATED_USTRUCT_BODY();

	// amount of damage actually applied
	UPROPERTY()
	float ActualDamageTaken;

	// type of damage that was applied
	UPROPERTY()
	UClass* DamageTypeClass;

	// who hit us
	UPROPERTY()
	TWeakObjectPtr<class ANativeBaseCharacter> InstigatorPawn;

	// what caused the damage
	UPROPERTY()
	TWeakObjectPtr<class AActor> DamageCauser;

	// style of damage application
	UPROPERTY()
	uint32 DamageStyleID;

	// hit result
	UPROPERTY()
	EHitResult HitResult;

private:
	// rolling counter to ensure struct is dirty and thus force replication
	UPROPERTY()
	uint8 m_ensureReplicationByte;

	// data for general damage
	FDamageEvent m_generalDamageEvent;

	// data for point damage
	FPointDamageEvent m_pointDamageEvent;

	// data for radial damage
	FRadialDamageEvent m_radialDamageEvent;

public:
	FTakeHitInfo();

	const FDamageEvent& GetDamageEvent();
	void SetDamageEvent(const FDamageEvent& damageEvent);
	void ForceReplication();	
};
