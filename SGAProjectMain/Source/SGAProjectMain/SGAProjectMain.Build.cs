// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SGAProjectMain : ModuleRules
{
	public SGAProjectMain(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "NavigationSystem", "AIModule", "GameplayTasks", "Niagara", "SlateCore", "MotionWarping" });
    }
}
