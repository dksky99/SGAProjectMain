#include "UpperBodyAimPatchCommandlet.h"
#include "AnimGraphNode_UpperBodyAim.h"
#include "AnimGraphNode_LegIK.h"
#include "AnimGraphNode_ComponentToLocalSpace.h"
#include "Animation/AnimBlueprint.h"
#include "EdGraph/EdGraph.h"
#include "K2Node_VariableGet.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/CompilerResultsLog.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "UObject/SavePackage.h"
#include "UObject/UnrealType.h"

UUpperBodyAimPatchCommandlet::UUpperBodyAimPatchCommandlet()
{
    IsClient=false; IsServer=false; IsEditor=true; LogToConsole=true;
}
int32 UUpperBodyAimPatchCommandlet::Main(const FString& Params)
{
    auto* BP = LoadObject<UAnimBlueprint>(nullptr,TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/ABP_Helldiver_MK5.ABP_Helldiver_MK5"));
    if (!BP) return 1;
    FGuid LegGuid, ConvertGuid;
    FGuid::Parse(TEXT("A7FF3CB1431D17B401778386B4A2065F"), LegGuid);
    FGuid::Parse(TEXT("123FE6304D4CE11A6B4ADFBD225A90BA"), ConvertGuid);
    UAnimGraphNode_LegIK* Legs=nullptr;
    UAnimGraphNode_ComponentToLocalSpace* Conversion=nullptr;
    TArray<UEdGraph*> Graphs; BP->GetAllGraphs(Graphs);
    for (UEdGraph* G : Graphs)
        for (UEdGraphNode* N : G->Nodes)
        {
            if (Cast<UAnimGraphNode_UpperBodyAim>(N)) return 2; // Never silently stack corrections.
            if (N->NodeGuid==LegGuid) Legs=Cast<UAnimGraphNode_LegIK>(N);
            if (N->NodeGuid==ConvertGuid) Conversion=Cast<UAnimGraphNode_ComponentToLocalSpace>(N);
        }
    if (!Legs || !Conversion || Legs->GetGraph()!=Conversion->GetGraph()) return 3;
    UEdGraphPin* Source=Legs->FindPin(TEXT("Pose"), EGPD_Output);
    UEdGraphPin* Sink=Conversion->FindPin(TEXT("ComponentPose"), EGPD_Input);
    if (!Source || !Sink || Source->LinkedTo.Num()!=1 || Source->LinkedTo[0]!=Sink || Sink->LinkedTo.Num()!=1) return 4;
    const TCHAR* Inputs[][2] = {
        {TEXT("TargetPosition"),TEXT("_targetPos_Mesh")},
        {TEXT("MuzzleInHand"),TEXT("_weaponOffset")},
        {TEXT("BodyForward"),TEXT("_aimBodyForwardMesh")},
        {TEXT("BodyUp"),TEXT("_aimBodyUpMesh")},
        {TEXT("LeftGripInHand"),TEXT("_upperBodyLeftGripInHand")},
        {TEXT("LeftHandAlpha"),TEXT("_upperBodyLeftHandAlpha")},
        {TEXT("Alpha"),TEXT("_upperBodyAimAlpha")}};
    for (const auto& Input : Inputs)
        if (!FindFProperty<FProperty>(BP->ParentClass, Input[1])) return 5;
    UEdGraph* Graph=Conversion->GetGraph();
    const auto* Schema=Graph->GetSchema();
    FGraphNodeCreator<UAnimGraphNode_UpperBodyAim> Create(*Graph);
    UAnimGraphNode_UpperBodyAim* Aim=Create.CreateNode(false);
    Aim->NodePosX=Legs->NodePosX+260; Aim->NodePosY=Legs->NodePosY;
    Aim->NodeComment=TEXT("Bounded spine rotation, evaluated-pose muzzle stabilization and same-frame left grip. Prone bypasses this node.");
    Create.Finalize();
    Source->BreakLinkTo(Sink);
    if (!Schema->TryCreateConnection(Source,Aim->FindPin(TEXT("ComponentPose"),EGPD_Input)) ||
        !Schema->TryCreateConnection(Aim->FindPin(TEXT("Pose"),EGPD_Output),Sink)) return 6;
    for (int32 I=0; I<UE_ARRAY_COUNT(Inputs); ++I)
    {
        FGraphNodeCreator<UK2Node_VariableGet> C(*Graph);
        auto* Getter=C.CreateNode(false);
        Getter->SetFromProperty(FindFProperty<FProperty>(BP->ParentClass,Inputs[I][1]),true,BP->ParentClass);
        Getter->NodePosX=Aim->NodePosX-300; Getter->NodePosY=Aim->NodePosY+150+I*90;
        C.Finalize();
        if (!Schema->TryCreateConnection(Getter->GetValuePin(),Aim->FindPin(Inputs[I][0],EGPD_Input))) return 7;
    }
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
    FCompilerResultsLog Results;
    FKismetEditorUtilities::CompileBlueprint(BP,EBlueprintCompileOptions::SkipGarbageCollection,&Results);
    UE_LOG(LogTemp,Display,TEXT("UpperBodyAim compile: errors=%d warnings=%d node=%s"), Results.NumErrors,Results.NumWarnings,*Aim->NodeGuid.ToString());
    if (Results.NumErrors || Results.NumWarnings) return 8;
    const FString File=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("UpperBodyAimPatch/ABP_Helldiver_MK5.uasset"));
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(File),true);
    FSavePackageArgs Save; Save.TopLevelFlags=RF_Public|RF_Standalone; Save.SaveFlags=SAVE_NoError;
    if (!UPackage::SavePackage(BP->GetOutermost(),BP,*File,Save)) return 9;
    UE_LOG(LogTemp,Display,TEXT("Staged %s"),*File);
    return 0;
}
