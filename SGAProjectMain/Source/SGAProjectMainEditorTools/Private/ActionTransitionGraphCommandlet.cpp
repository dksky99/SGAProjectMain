#include "ActionTransitionGraphCommandlet.h"
#include "AnimGraphNode_RotationOffsetBlendSpace.h"
#include "Animation/AnimBlueprint.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_VariableGet.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/CompilerResultsLog.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UObject/SavePackage.h"
#include "UObject/UnrealType.h"

UActionTransitionGraphCommandlet::UActionTransitionGraphCommandlet()
{
    IsClient = false; IsServer = false; IsEditor = true; LogToConsole = true;
}

int32 UActionTransitionGraphCommandlet::Main(const FString& Params)
{
    // Save candidates under Saved only. Applying them to Content is a separate step.
    auto* Blueprint = LoadObject<UAnimBlueprint>(nullptr,
        TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/ABP_Helldiver_Layer.ABP_Helldiver_Layer"));
    if (!Blueprint || !FindFProperty<FFloatProperty>(Blueprint->ParentClass, TEXT("_actionAimAlpha"))) return 1;
    TArray<UEdGraph*> Graphs;
    Blueprint->GetAllGraphs(Graphs);
    UAnimGraphNode_RotationOffsetBlendSpace* Aim = nullptr;
    const FGuid Expected(0x57BEBA77,0x4FB3F4C1,0x6349D79D,0x1B0CCFD9);
    for (UEdGraph* Graph : Graphs)
        for (UEdGraphNode* Node : Graph->Nodes)
            if (Node && Node->NodeGuid == Expected) Aim = Cast<UAnimGraphNode_RotationOffsetBlendSpace>(Node);
    if (!Aim) return 2;
    UEdGraphPin* Alpha = Aim->FindPin(TEXT("Alpha"), EGPD_Input);
    if (!Alpha || Alpha->LinkedTo.Num() != 0) return 3;
    FGraphNodeCreator<UK2Node_VariableGet> Creator(*Aim->GetGraph());
    UK2Node_VariableGet* Getter = Creator.CreateNode(false);
    Getter->SetFromProperty(FindFProperty<FFloatProperty>(Blueprint->ParentClass, TEXT("_actionAimAlpha")), true, Blueprint->ParentClass);
    Getter->NodePosX = Aim->NodePosX - 300;
    Getter->NodePosY = Aim->NodePosY + 200;
    Getter->NodeComment = TEXT("Action transitions suppress AO while a posture/weapon montage is active.");
    Creator.Finalize();
    const UEdGraphSchema* Schema = Aim->GetGraph()->GetSchema();
    if (!Schema->TryCreateConnection(Getter->GetValuePin(), Alpha)) return 4;
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
    const TCHAR* Names[] = {TEXT("ABP_Helldiver_Layer"),TEXT("ABP_Helldiver_Rifle"),TEXT("ABP_Helldiver_Pistol"),TEXT("ABP_Helldiver_Unarmed")};
    for (const TCHAR* Name : Names)
    {
        const FString Path = FString::Printf(TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/%s.%s"), Name, Name);
        UBlueprint* Current = LoadObject<UBlueprint>(nullptr, *Path);
        if (!Current) return 5;
        FCompilerResultsLog Results;
        FKismetEditorUtilities::CompileBlueprint(Current, EBlueprintCompileOptions::SkipGarbageCollection, &Results);
        UE_LOG(LogTemp, Display, TEXT("Action transition graph %s: errors=%d warnings=%d"), Name, Results.NumErrors, Results.NumWarnings);
        if (Results.NumErrors || Results.NumWarnings) return 6;
        const FString File = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("ActionTransitionGraph") / (FString(Name) + TEXT(".uasset")));
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(File), true);
        FSavePackageArgs Save;
        Save.TopLevelFlags = RF_Public | RF_Standalone; Save.SaveFlags = SAVE_NoError;
        if (!UPackage::SavePackage(Current->GetOutermost(), Current, *File, Save)) return 7;
        UE_LOG(LogTemp, Display, TEXT("Staged %s"), *File);
    }
    return 0;
}
