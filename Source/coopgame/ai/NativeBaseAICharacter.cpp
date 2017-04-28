// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "NativeBaseAICharacter.h"

ANativeBaseAICharacter::ANativeBaseAICharacter(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	bUseControllerRotationYaw = true;
}
