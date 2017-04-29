// Fill out your copyright notice in the Description page of Project Settings.

#include "CoopGame.h"
#include "CoopTypes.h"
#include "NativeBaseCharacter.h"

FTakeHitInfo::FTakeHitInfo()
	: ActualDamageTaken(0.0f)
	, DamageTypeClass(nullptr)
	, InstigatorPawn(nullptr)
	, DamageCauser(nullptr)
	, DamageStyleID(0)
	, HitResult(EHitResult::Wounded)
	, m_ensureReplicationByte(0)
{
}

const FDamageEvent& FTakeHitInfo::GetDamageEvent()
{
	switch (DamageStyleID)
	{
	case FPointDamageEvent::ClassID:
		if (m_pointDamageEvent.DamageTypeClass == nullptr)
			m_pointDamageEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : UDamageType::StaticClass();
		return m_pointDamageEvent;
	case FRadialDamageEvent::ClassID:
		if (m_radialDamageEvent.DamageTypeClass == nullptr)
			m_radialDamageEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : UDamageType::StaticClass();
		return m_radialDamageEvent;
	default:
		if (m_generalDamageEvent.DamageTypeClass == nullptr)
			m_generalDamageEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : UDamageType::StaticClass();
		return m_generalDamageEvent;
	}
}

void FTakeHitInfo::SetDamageEvent(const FDamageEvent& damageEvent)
{
	DamageStyleID = damageEvent.GetTypeID();
	switch(DamageStyleID)
	{
	case FPointDamageEvent::ClassID:
		m_pointDamageEvent = *((const FPointDamageEvent*)(&damageEvent));
		break;
	case FRadialDamageEvent::ClassID:
		m_radialDamageEvent = *((const FRadialDamageEvent*)(&damageEvent));
		break;
	default:
		m_generalDamageEvent = damageEvent;
		break;
	}

	DamageTypeClass = damageEvent.DamageTypeClass;
}

void FTakeHitInfo::ForceReplication()
{
	++m_ensureReplicationByte;
}
