// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class coopgame : ModuleRules
{
	public coopgame(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;


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
                "NavigationSystem",
                "AIModule",
                "GameplayTasks",
            }
        );
        DynamicallyLoadedModuleNames.AddRange(
            new string[] {
                "OnlineSubsystemSteam",
            }
        );
    }
}
