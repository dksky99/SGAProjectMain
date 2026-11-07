using UnrealBuildTool;

public class SGAProjectMainEditorTools : ModuleRules
{
    public SGAProjectMainEditorTools(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        // Graph-node UObjects are needed to compile uncooked animation assets,
        // while the evaluated FAnimNode lives in the separate runtime module.
        OverridePackageType = PackageOverrideType.GameUncookedOnly;
        PrivateDependencyModuleNames.AddRange(new[]
        {
            "Core", "CoreUObject", "Engine", "UnrealEd", "BlueprintGraph",
            "Kismet", "KismetCompiler", "AnimGraph", "AnimGraphRuntime",
            "AssetRegistry", "Json", "SGAProjectMain", "RigVM", "RigVMDeveloper",
            "ControlRig", "ControlRigDeveloper", "EditorWidgets", "Slate", "SlateCore"
        });
    }
}
