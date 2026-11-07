#include "CoreMinimal.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "../AnimGraphNode_UpperBodyAim.h"
#include "AnimGraphNode_LegIK.h"
#include "AnimGraphNode_ComponentToLocalSpace.h"
#include "AnimGraphNode_Root.h"
#include "Animation/AnimBlueprint.h"
#include "EdGraph/EdGraph.h"
#include "K2Node_VariableGet.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUpperBodyAimGraphBindingsTest, "SGA.Character.UpperBodyAim.GraphBindings",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FUpperBodyAimGraphBindingsTest::RunTest(const FString& Parameters)
{
    auto* BP = LoadObject<UAnimBlueprint>(nullptr,
        TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/ABP_Helldiver_MK5.ABP_Helldiver_MK5"));
    if (!TestNotNull(TEXT("The actual MK5 animation Blueprint loads"),BP)) return false;
    FGuid LegGuid, ConvertGuid;
    FGuid::Parse(TEXT("A7FF3CB1431D17B401778386B4A2065F"), LegGuid);
    FGuid::Parse(TEXT("123FE6304D4CE11A6B4ADFBD225A90BA"), ConvertGuid);
    UAnimGraphNode_UpperBodyAim* Aim = nullptr;
    UAnimGraphNode_LegIK* Leg = nullptr;
    UAnimGraphNode_ComponentToLocalSpace* Conversion = nullptr;
    int32 AimCount = 0;
    TArray<UEdGraph*> Graphs; BP->GetAllGraphs(Graphs);
    for (UEdGraph* Graph : Graphs)
    for (UEdGraphNode* Node : Graph->Nodes)
    {
        if (auto* Candidate = Cast<UAnimGraphNode_UpperBodyAim>(Node)) { Aim=Candidate; ++AimCount; }
        if (Node->NodeGuid==LegGuid) Leg=Cast<UAnimGraphNode_LegIK>(Node);
        if (Node->NodeGuid==ConvertGuid) Conversion=Cast<UAnimGraphNode_ComponentToLocalSpace>(Node);
    }
    TestEqual(TEXT("The final graph contains exactly one upper-body correction"),AimCount,1);
    if (!TestNotNull(TEXT("The correction exists"),Aim) || !TestNotNull(TEXT("The original final conversion exists"),Conversion)
        || !TestNotNull(TEXT("The original LegIK exists"),Leg)) return false;
    const auto Connected = [](UEdGraphPin* A,UEdGraphPin* B)
    { return A && B && A->LinkedTo.Num()==1 && B->LinkedTo.Num()==1 && A->LinkedTo[0]==B && B->LinkedTo[0]==A; };
    TestTrue(TEXT("Final foot and LegIK pose enters the correction directly"),
        Connected(Leg->FindPin(TEXT("Pose"),EGPD_Output),Aim->FindPin(TEXT("ComponentPose"),EGPD_Input)));
    TestTrue(TEXT("The corrected current pose feeds the final local conversion"),
        Connected(Aim->FindPin(TEXT("Pose"),EGPD_Output),Conversion->FindPin(TEXT("ComponentPose"),EGPD_Input)));
    UEdGraphPin* FinalPose=Conversion->FindPin(TEXT("Pose"),EGPD_Output);
    if (TestNotNull(TEXT("The final local pose output exists"),FinalPose)
        && TestEqual(TEXT("Final conversion has one consumer"),FinalPose->LinkedTo.Num(),1))
    {
        auto* Result=Cast<UAnimGraphNode_Root>(FinalPose->LinkedTo[0]->GetOwningNode());
        TestNotNull(TEXT("No later skeletal stage moves the stabilized muzzle"),Result);
        if (Result) TestTrue(TEXT("The local conversion directly supplies the animation Result"),
            Connected(FinalPose,Result->FindPin(TEXT("Result"),EGPD_Input)));
    }
    const TCHAR* Inputs[][2] = {
        {TEXT("TargetPosition"),TEXT("_targetPos_Mesh")},
        {TEXT("MuzzleInHand"),TEXT("_weaponOffset")},
        {TEXT("BodyForward"),TEXT("_aimBodyForwardMesh")},
        {TEXT("BodyUp"),TEXT("_aimBodyUpMesh")},
        {TEXT("Alpha"),TEXT("_upperBodyAimAlpha")},
        {TEXT("LeftGripInHand"),TEXT("_upperBodyLeftGripInHand")},
        {TEXT("LeftHandAlpha"),TEXT("_upperBodyLeftHandAlpha")}};
    TSet<UEdGraphNode*> GetterNodes;
    for (const auto& Input : Inputs)
    {
        UEdGraphPin* Pin=Aim->FindPin(Input[0],EGPD_Input);
        if (!TestNotNull(FString::Printf(TEXT("%s input is exposed"),Input[0]),Pin)) continue;
        if (!TestEqual(FString::Printf(TEXT("%s has exactly one binding"),Input[0]),Pin->LinkedTo.Num(),1)) continue;
        auto* Getter=Cast<UK2Node_VariableGet>(Pin->LinkedTo[0]->GetOwningNode());
        if (!TestNotNull(FString::Printf(TEXT("%s reads a variable getter"),Input[0]),Getter)) continue;
        TestEqual(FString::Printf(TEXT("%s reads the intended snapshot"),Input[0]),Getter->VariableReference.GetMemberName(),FName(Input[1]));
        GetterNodes.Add(Getter);
    }
    TestEqual(TEXT("Seven distinct direct getter bindings were installed once"),GetterNodes.Num(),7);
    return true;
}
#endif
