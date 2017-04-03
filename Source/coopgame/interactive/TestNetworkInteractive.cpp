// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "TestNetworkInteractive.h"
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>

// Sets default values
ATestNetworkInteractive::ATestNetworkInteractive()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATestNetworkInteractive::BeginPlay()
{
	Super::BeginPlay();	
}

// Called every frame
void ATestNetworkInteractive::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATestNetworkInteractive::DoStuff_Implementation(UObject* obj)
{
	if (obj)
	{
		OnStuffDone();
	}
}