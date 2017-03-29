// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class coopgame : ModuleRules
{
	public coopgame(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
        DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
	}
}
