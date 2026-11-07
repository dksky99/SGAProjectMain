#include "ProneHeadAnchorGraphPatch.h"
#include "AnimGraphNode_ProneHeadAnchor.h"
#include "AnimGraphNode_LegIK.h"
#include "AnimGraphNode_ComponentToLocalSpace.h"
#include "Engine/Blueprint.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphSchema.h"
#include "Kismet2/BlueprintEditorUtils.h"

namespace CharacterAnimationAudit
{
    bool CompileForAudit(UBlueprint* Blueprint, TSharedRef<FJsonObject> Result);
    bool SaveStagedBlueprint(UBlueprint* Blueprint, TSharedRef<FJsonObject> Result,
        TFunctionRef<bool()> WriteCheckpoint);

    bool RemoveProneHeadAnchor(UBlueprint* Blueprint, TSharedRef<FJsonObject> Result,
        TFunctionRef<bool()> WriteCheckpoint)
    {
        const auto Fail = [&Result](const TCHAR* Message)
        {
            Result->SetStringField(TEXT("error"), Message);
            UE_LOG(LogTemp, Error, TEXT("ProneHeadAnchorGraphPatch: %s"), Message);
            return false;
        };
        if (!Blueprint || Blueprint->GetPathName() != TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/ABP_Helldiver_MK5.ABP_Helldiver_MK5"))
            return Fail(TEXT("Unexpected main animation Blueprint; no asset changed."));
        FGuid LegGuid, ConversionGuid, AnchorGuid;
        FGuid::Parse(TEXT("A7FF3CB1431D17B401778386B4A2065F"), LegGuid);
        FGuid::Parse(TEXT("123FE6304D4CE11A6B4ADFBD225A90BA"), ConversionGuid);
        FGuid::Parse(TEXT("0E40F3884538B45994F4C9912C61838F"), AnchorGuid);
        UAnimGraphNode_LegIK* Leg = nullptr;
        UAnimGraphNode_ComponentToLocalSpace* Conversion = nullptr;
        UAnimGraphNode_ProneHeadAnchor* Anchor = nullptr;
        int32 AnchorCount = 0;
        int32 LegCount = 0, ConversionCount = 0, KnownAnchorCount = 0;
        TArray<UEdGraph*> Graphs;
        Blueprint->GetAllGraphs(Graphs);
        for (UEdGraph* Graph : Graphs)
            for (UEdGraphNode* Node : Graph->Nodes)
            {
                if (Node->NodeGuid == LegGuid) { Leg = Cast<UAnimGraphNode_LegIK>(Node); ++LegCount; }
                if (Node->NodeGuid == ConversionGuid) { Conversion = Cast<UAnimGraphNode_ComponentToLocalSpace>(Node); ++ConversionCount; }
                if (Node->NodeGuid == AnchorGuid) { Anchor = Cast<UAnimGraphNode_ProneHeadAnchor>(Node); ++KnownAnchorCount; }
                if (Cast<UAnimGraphNode_ProneHeadAnchor>(Node)) ++AnchorCount;
            }
        if (!Leg || !Conversion || LegCount != 1 || ConversionCount != 1 ||
            Leg->GetGraph() != Conversion->GetGraph() || AnchorCount > 1 || KnownAnchorCount > 1 ||
            (KnownAnchorCount != 0 && !Anchor) || (AnchorCount != 0 && !Anchor))
            return Fail(TEXT("The expected final component-space chain was not found."));
        UEdGraph* Graph = Leg->GetGraph();
        UEdGraphPin* Source = Leg->FindPin(TEXT("Pose"), EGPD_Output);
        UEdGraphPin* Sink = Conversion->FindPin(TEXT("ComponentPose"), EGPD_Input);
        if (!Source || !Sink) return Fail(TEXT("The expected pose pins were not found."));
        const auto IsLinked = [](UEdGraphPin* A, UEdGraphPin* B)
        { return A && B && A->LinkedTo.Num() == 1 && B->LinkedTo.Num() == 1 && A->LinkedTo[0] == B && B->LinkedTo[0] == A; };
        if (!Anchor)
        {
            if (!IsLinked(Source, Sink))
                return Fail(TEXT("No known anchor exists and the final chain is not the expected direct connection; no package saved."));
            Result->SetBoolField(TEXT("already_removed"), true);
            Result->SetBoolField(TEXT("graph_changed"), false);
        }
        else
        {
            UEdGraphPin* AnchorInput = Anchor->FindPin(TEXT("ComponentPose"), EGPD_Input);
            UEdGraphPin* AnchorOutput = Anchor->FindPin(TEXT("Pose"), EGPD_Output);
            if (Anchor->GetGraph() != Graph || !IsLinked(Source, AnchorInput) || !IsLinked(AnchorOutput, Sink))
                return Fail(TEXT("The known head anchor has unexpected connections; no package saved."));
            for (UEdGraphPin* Pin : Anchor->Pins)
                if (Pin && Pin != AnchorInput && Pin != AnchorOutput && !Pin->LinkedTo.IsEmpty())
                    return Fail(TEXT("The known head anchor has additional linked inputs or outputs; no package saved."));

            Blueprint->Modify();
            Graph->Modify();
            Leg->Modify();
            Conversion->Modify();
            Anchor->Modify();
            Source->BreakLinkTo(AnchorInput);
            AnchorOutput->BreakLinkTo(Sink);
            if (!Graph->GetSchema()->TryCreateConnection(Source, Sink))
            {
                Source->MakeLinkTo(AnchorInput);
                AnchorOutput->MakeLinkTo(Sink);
                return Fail(TEXT("Could not restore the direct final pose chain; no package saved."));
            }
            if (!Graph->RemoveNode(Anchor))
            {
                Source->BreakLinkTo(Sink);
                Source->MakeLinkTo(AnchorInput);
                AnchorOutput->MakeLinkTo(Sink);
                return Fail(TEXT("Could not remove the known head anchor; no package saved."));
            }
            FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
            Result->SetBoolField(TEXT("already_removed"), false);
            Result->SetBoolField(TEXT("graph_changed"), true);
        }
        if (!IsLinked(Source, Sink))
            return Fail(TEXT("The restored final pose chain is not a reciprocal direct connection; no package saved."));
        for (UEdGraph* CandidateGraph : Graphs)
            for (UEdGraphNode* Node : CandidateGraph->Nodes)
                if (Cast<UAnimGraphNode_ProneHeadAnchor>(Node) || Node->NodeGuid == AnchorGuid)
                    return Fail(TEXT("A head anchor remains in the Blueprint; no package saved."));
        Result->SetStringField(TEXT("removed_anchor_node_guid"), AnchorGuid.ToString(EGuidFormats::Digits));
        Result->SetStringField(TEXT("source_leg_ik_guid"), LegGuid.ToString(EGuidFormats::Digits));
        Result->SetStringField(TEXT("sink_component_to_local_guid"), ConversionGuid.ToString(EGuidFormats::Digits));
        Result->SetStringField(TEXT("policy"), TEXT("Retire head pinning; direct LegIK Pose to ComponentToLocal ComponentPose; authored animation remains unconstrained."));
        if (!CompileForAudit(Blueprint, Result)) return Fail(TEXT("Restored graph compilation failed; no package saved."));
        return SaveStagedBlueprint(Blueprint, Result, WriteCheckpoint);
    }
}
