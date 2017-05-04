// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class coopgame : ModuleRules
{
	public coopgame(TargetInfo Target)
	{
		Definitions.Add("_CRT_SECURE_NO_WARNINGS");
		PublicDependencyModuleNames.AddRange(
			new string[] { 
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"HeadMountedDisplay",
				"UMG",
				"Slate",
				"SlateCore",
				"OnlineSubsystem",
				"OnlineSubsystemUtils",
			}
		);
		DynamicallyLoadedModuleNames.AddRange(
			new string[] {
				"OnlineSubsystemSteam",
			}
		);
	}
}
