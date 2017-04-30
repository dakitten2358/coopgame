// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "NativeProjectileBase.h"
#include "NativeBaseCharacter.h"

ANativeProjectileBase::ANativeProjectileBase(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	// collision
	m_collisionComponent = objectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("SphereComponent"));
	m_collisionComponent->InitSphereRadius(5.0f); // ??
	m_collisionComponent->AlwaysLoadOnClient = true;
	m_collisionComponent->AlwaysLoadOnServer = true;
	m_collisionComponent->bTraceComplexOnMove = true;
	m_collisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	m_collisionComponent->SetCollisionObjectType(COLLISION_PROJECTILE);
	m_collisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	m_collisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	m_collisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	m_collisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	RootComponent = m_collisionComponent;

	// movement
	m_movementComponent = objectInitializer.CreateDefaultSubobject<UProjectileMovementComponent>(this, TEXT("ProjectileMovementComponent"));
	m_movementComponent->UpdatedComponent = m_collisionComponent;
	m_movementComponent->InitialSpeed = 2000.0f;
	m_movementComponent->MaxSpeed = 2000.0f;
	m_movementComponent->bRotationFollowsVelocity = true;
	m_movementComponent->ProjectileGravityScale = 0.f;
	
	// ticking
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	// network
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bReplicateMovement = true;
}

void ANativeProjectileBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	m_movementComponent->OnProjectileStop.AddDynamic(this, &ANativeProjectileBase::OnImpact);
	m_collisionComponent->MoveIgnoreActors.Add(Instigator);

	//AShooterWeapon_Projectile* OwnerWeapon = Cast<AShooterWeapon_Projectile>(GetOwner());
	//if (OwnerWeapon)
	//{
	//	OwnerWeapon->ApplyWeaponConfig(WeaponConfig);
	//}

	//SetLifeSpan(WeaponConfig.ProjectileLife);
	//MyController = GetInstigatorController();
}

void ANativeProjectileBase::OnImpact(const FHitResult& hitResult)
{
	if (Role == ROLE_Authority/* && !bExploded*/)
	{
		//Explode(HitResult);
		if (hitResult.Actor != nullptr)
		{
			auto actor = hitResult.Actor.Get();
			UE_LOG(LogCoopGameWeapon, Log, TEXT("Hit an actor!"));
			auto asCharacter = Cast<ANativeBaseCharacter>(actor);
			if (asCharacter != nullptr)
			{
				UE_LOG(LogCoopGameWeapon, Log, TEXT("Hit a coop character"));

				UGameplayStatics::ApplyDamage(asCharacter, 25.0f, Instigator->Controller, this, TSubclassOf<UDamageType>(UDamageType::StaticClass()));
			}
		}
		else
		{
			UE_LOG(LogCoopGameWeapon, Log, TEXT("Hit something else"));
		}
		DisableAndDestroy();
	}

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), m_impactEffects, GetActorLocation(), GetActorRotation().GetInverse());

	OnProjectileImpacted();
}

void ANativeProjectileBase::DisableAndDestroy()
{
	/*UAudioComponent* ProjAudioComp = FindComponentByClass<UAudioComponent>();
	if (ProjAudioComp && ProjAudioComp->IsPlaying())
	{
		ProjAudioComp->FadeOut(0.1f, 0.f);
	}*/

	m_movementComponent->StopMovementImmediately();
	SetActorEnableCollision(false);

	// give clients some time to show explosion
	SetLifeSpan(2.0f);
}

void ANativeProjectileBase::OnProjectileImpacted_Implementation()
{
	// default impl does nothing
}
