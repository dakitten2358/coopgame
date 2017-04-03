// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "TestNetworkInteractive.generated.h"

UCLASS()
class COOPGAME_API ATestNetworkInteractive : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATestNetworkInteractive();

	UFUNCTION(Reliable, NetMulticast, BlueprintCallable)
	void DoStuff(UObject* obj);
	void DoStuff_Implementation(UObject* obj);

	UFUNCTION(BlueprintImplementableEvent)
	void OnStuffDone();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
