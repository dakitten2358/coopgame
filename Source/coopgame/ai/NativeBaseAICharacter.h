// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeBaseCharacter.h"
#include "NativeBaseAICharacter.generated.h"

UCLASS()
class COOPGAME_API ANativeBaseAICharacter : public ANativeBaseCharacter
{
	GENERATED_UCLASS_BODY()
	
	UPROPERTY(EditAnywhere, Category = Behavior)
	class UBehaviorTree* AIBehavior;	
};
