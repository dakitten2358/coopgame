// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "NativeProjectileBase.generated.h"

UCLASS(Abstract, Blueprintable)
class COOPGAME_API ANativeProjectileBase : public AActor
{
	GENERATED_UCLASS_BODY()

	virtual void PostInitializeComponents() override;

	UFUNCTION()
	void OnImpact(const FHitResult& HitResult);
	
private:
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	UProjectileMovementComponent* m_movementComponent;

	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	USphereComponent* m_collisionComponent;

private:
	void DisableAndDestroy();
	
};
