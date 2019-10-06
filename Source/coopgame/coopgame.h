// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

// logging
// --------------------------------------------------------------------------
DECLARE_LOG_CATEGORY_EXTERN(LogCoopGame, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogCoopGameOnline, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogCoopGameWeapon, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogCoopGameNotImplemented, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogCoopGameTodo, Log, All);


// collision channels
// --------------------------------------------------------------------------
// be aware that these values can be saved into an instance
// DefaultEngine.ini [/Script/Engine.CollisionProfile] should match this list
// also found under Project Settings > Engine > Collision > Trace Channels
#define COLLISION_WEAPON				ECC_GameTraceChannel1
#define COLLISION_PROJECTILE			ECC_GameTraceChannel2
