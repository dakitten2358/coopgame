// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "NativeProjectileBase.generated.h"

UCLASS(Abstract, Blueprintable)
class ANativeProjectileBase : public AActor
{
	GENERATED_UCLASS_BODY()

	virtual void PostInitializeComponents() override;

	UFUNCTION()
	void OnImpact(const FHitResult& hitResult);

	UFUNCTION(BlueprintNativeEvent)
	void OnProjectileImpacted();
	
private:
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	UProjectileMovementComponent* m_movementComponent;

	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	USphereComponent* m_collisionComponent;

	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	UParticleSystem* m_impactEffects;

private:
	void DisableAndDestroy();
	
};
