// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class SGAProjectMain : ModuleRules
{
	public SGAProjectMain(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "NavigationSystem", "AIModule", "AnimGraphRuntime", "GameplayTasks", "Niagara", "SlateCore", "MotionWarping", "GeometryCollectionEngine", "ChaosSolverEngine" });

    

    }
}
