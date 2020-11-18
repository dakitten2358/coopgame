// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class coopgameServerTarget : TargetRules
{
	public coopgameServerTarget(TargetInfo Target) : base(Target)
	{
		DefaultBuildSettings = BuildSettingsVersion.V2;
		Type = TargetType.Server;
		ExtraModuleNames.Add("coopgame");
		ExtraModuleNames.Add("CoopFramework");
	}
}
