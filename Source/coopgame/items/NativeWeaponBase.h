// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "NativeWeaponBase.generated.h"

class ANativeCoopCharacter;

UCLASS()
class COOPGAME_API ANativeWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANativeWeaponBase(const FObjectInitializer& objectInitializer);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Weapon Mesh
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// INVENTORY RELATED
	// -----------------------------------------------------------------------------
public:
	void OnEnterInventory(ANativeCoopCharacter*);
	void OnLeaveInventory();

protected:
	/** pawn owner */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_OwningCharacter)
	ANativeCoopCharacter* OwningCharacter;

	UFUNCTION()
	void OnRep_OwningCharacter();

private:
	void SetOwningCharacter(ANativeCoopCharacter* newOwner);	
	void ResetOwningCharacter();

	void AttachMeshToCharacter();
	void DetachMeshFromCharacter();	
};
