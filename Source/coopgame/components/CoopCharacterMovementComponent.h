// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "CoopCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API UCoopCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	virtual float GetMaxSpeed() const override;	
};
