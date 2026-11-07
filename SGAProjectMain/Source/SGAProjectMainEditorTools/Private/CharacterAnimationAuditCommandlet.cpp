#include "CharacterAnimationAuditCommandlet.h"
#include "CharacterAnimationPlaybackVerify.h"
#include "ProneHeadAnchorGraphPatch.h"
#include "ProneAssetGeometry.h"
#include "ProneCanonicalAssets.h"
#include "ProneRollAssetPatch.h"
#include "ProneUnifiedAssets.h"

#include "AnimGraphNode_LayeredBoneBlend.h"
#include "AnimGraphNode_TwoBoneIK.h"
#include "AnimGraphNode_ControlRig.h"
#include "AnimGraphNode_Root.h"
#include "AnimGraphNode_SequencePlayer.h"
#include "AnimGraphNode_SequenceEvaluator.h"
#include "Animation/BlendProfile.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimSequence.h"
#include "Dom/JsonObject.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "Animation/AnimBlueprint.h"
#include "Engine/Blueprint.h"
#include "HAL/FileManager.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Variable.h"
#include "K2Node_VariableGet.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/CompilerResultsLog.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Logging/TokenizedMessage.h"
#include "Misc/EngineVersion.h"
#include "Misc/FileHelper.h"
#include "Misc/FeedbackContext.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "RigVMBlueprint.h"
#include "RigVMModel/Nodes/RigVMCollapseNode.h"
#include "RigVMModel/RigVMFunctionLibrary.h"
#include "RigVMModel/RigVMGraph.h"
#include "RigVMModel/RigVMLink.h"
#include "RigVMModel/RigVMNode.h"
#include "RigVMModel/RigVMPin.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/NoExportTypes.h"
#include "UObject/Linker.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

namespace CharacterAnimationAudit
{
    using FObject = TSharedRef<FJsonObject>;
    using FValues = TArray<TSharedPtr<FJsonValue>>;
    static const TCHAR* MainBlueprint = TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/ABP_Helldiver_MK5");

    bool WriteReport(FObject Report, const FString& ReportPath)
    {
        FString Json;
        const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Json);
        if (!FJsonSerializer::Serialize(Report, Writer)) return false;
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(ReportPath), true);
        return FFileHelper::SaveStringToFile(Json, *ReportPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    }

    FValues Properties(const UObject* Object, uint32 PortFlags = PPF_PropertyWindow)
    {
        FValues Values;
        if (!Object) return Values;
        for (TFieldIterator<FProperty> It(Object->GetClass()); It; ++It)
        {
            const FProperty* Property = *It;
            if (Property->HasAnyPropertyFlags(CPF_Transient | CPF_Deprecated)) continue;
            FString Value;
            Property->ExportTextItem_Direct(Value, Property->ContainerPtrToValuePtr<void>(Object),
                nullptr, const_cast<UObject*>(Object), PortFlags);
            FObject Row = MakeShared<FJsonObject>();
            Row->SetStringField(TEXT("name"), Property->GetName());
            Row->SetStringField(TEXT("type"), Property->GetCPPType());
            Row->SetBoolField(TEXT("truncated"), Value.Len() > 16384);
            Row->SetStringField(TEXT("value"), Value.Left(16384));
            Values.Add(MakeShared<FJsonValueObject>(Row));
        }
        return Values;
    }

    FObject Pin(const UEdGraphPin* Pin)
    {
        FObject Row = MakeShared<FJsonObject>();
        Row->SetStringField(TEXT("name"), Pin->PinName.ToString());
        Row->SetStringField(TEXT("id"), Pin->PinId.ToString());
        Row->SetStringField(TEXT("direction"), Pin->Direction == EGPD_Input ? TEXT("input") : TEXT("output"));
        Row->SetStringField(TEXT("category"), Pin->PinType.PinCategory.ToString());
        Row->SetStringField(TEXT("subcategory"), Pin->PinType.PinSubCategory.ToString());
        Row->SetStringField(TEXT("type_object"), GetPathNameSafe(Pin->PinType.PinSubCategoryObject.Get()));
        Row->SetStringField(TEXT("default_value"), Pin->DefaultValue);
        Row->SetStringField(TEXT("default_object"), GetPathNameSafe(Pin->DefaultObject));
        Row->SetStringField(TEXT("default_text"), Pin->DefaultTextValue.ToString());
        Row->SetBoolField(TEXT("hidden"), Pin->bHidden);
        Row->SetBoolField(TEXT("orphaned"), Pin->bOrphanedPin);
        FValues Links;
        for (const UEdGraphPin* Linked : Pin->LinkedTo)
        {
            if (!Linked) continue;
            FObject Link = MakeShared<FJsonObject>();
            Link->SetStringField(TEXT("node"), Linked->GetOwningNode()->GetPathName());
            Link->SetStringField(TEXT("node_guid"), Linked->GetOwningNode()->NodeGuid.ToString());
            Link->SetStringField(TEXT("pin"), Linked->PinName.ToString());
            Link->SetStringField(TEXT("pin_id"), Linked->PinId.ToString());
            Links.Add(MakeShared<FJsonValueObject>(Link));
        }
        Row->SetArrayField(TEXT("links"), Links);
        return Row;
    }

    FObject Node(UEdGraphNode* Node)
    {
        FObject Row = MakeShared<FJsonObject>();
        Row->SetStringField(TEXT("path"), Node->GetPathName());
        Row->SetStringField(TEXT("class"), Node->GetClass()->GetPathName());
        Row->SetStringField(TEXT("guid"), Node->NodeGuid.ToString());
        Row->SetStringField(TEXT("title"), Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
        Row->SetArrayField(TEXT("properties"), Properties(Node));
        if (const UK2Node_CallFunction* Call = Cast<UK2Node_CallFunction>(Node))
            Row->SetStringField(TEXT("function_reference"), Call->FunctionReference.GetMemberName().ToString());
        if (const UK2Node_Variable* Variable = Cast<UK2Node_Variable>(Node))
            Row->SetStringField(TEXT("variable_reference"), Variable->VariableReference.GetMemberName().ToString());
        if (const UAnimGraphNode_Base* Anim = Cast<UAnimGraphNode_Base>(Node))
        {
            FObject Callbacks = MakeShared<FJsonObject>();
            Callbacks->SetStringField(TEXT("initial_update"), Anim->InitialUpdateFunction.GetMemberName().ToString());
            Callbacks->SetStringField(TEXT("become_relevant"), Anim->BecomeRelevantFunction.GetMemberName().ToString());
            Callbacks->SetStringField(TEXT("update"), Anim->UpdateFunction.GetMemberName().ToString());
            Row->SetObjectField(TEXT("callbacks"), Callbacks);
            if (const FObjectPropertyBase* Binding = FindFProperty<FObjectPropertyBase>(Anim->GetClass(), TEXT("Binding")))
                Row->SetArrayField(TEXT("binding_properties"), Properties(Binding->GetObjectPropertyValue_InContainer(Anim), PPF_None));
        }
        FValues Pins;
        for (const UEdGraphPin* Item : Node->Pins)
            if (Item) Pins.Add(MakeShared<FJsonValueObject>(Pin(Item)));
        Row->SetArrayField(TEXT("pins"), Pins);
        if (const UAnimGraphNode_LayeredBoneBlend* Blend = Cast<UAnimGraphNode_LayeredBoneBlend>(Node))
        {
            FObject Settings = MakeShared<FJsonObject>();
            Settings->SetNumberField(TEXT("blend_mode"), static_cast<int32>(Blend->Node.BlendMode));
            Settings->SetNumberField(TEXT("blend_pose_count"), Blend->Node.BlendPoses.Num());
            Settings->SetBoolField(TEXT("mesh_space_rotation_blend"), Blend->Node.bMeshSpaceRotationBlend);
            Settings->SetBoolField(TEXT("mesh_space_scale_blend"), Blend->Node.bMeshSpaceScaleBlend);
            Settings->SetNumberField(TEXT("curve_blend_option"), static_cast<int32>(Blend->Node.CurveBlendOption.GetValue()));
            FValues Layers;
            for (const FInputBlendPose& Layer : Blend->Node.LayerSetup)
            {
                FValues Filters;
                for (const FBranchFilter& Filter : Layer.BranchFilters)
                {
                    FObject Item = MakeShared<FJsonObject>();
                    Item->SetStringField(TEXT("bone"), Filter.BoneName.ToString());
                    Item->SetNumberField(TEXT("depth"), Filter.BlendDepth);
                    Filters.Add(MakeShared<FJsonValueObject>(Item));
                }
                Layers.Add(MakeShared<FJsonValueArray>(Filters));
            }
            Settings->SetArrayField(TEXT("layer_setup"), Layers);
            FValues Masks;
            FValues MaskDetails;
            for (const UBlendProfile* Mask : Blend->Node.BlendMasks)
            {
                Masks.Add(MakeShared<FJsonValueString>(GetPathNameSafe(Mask)));
                FObject Detail = MakeShared<FJsonObject>();
                Detail->SetStringField(TEXT("path"), GetPathNameSafe(Mask));
                FValues Entries;
                if (Mask)
                {
                    Detail->SetNumberField(TEXT("default_scale"), Mask->GetDefaultBlendScale());
                    for (int32 Index = 0; Index < Mask->GetNumBlendEntries(); ++Index)
                    {
                        const FBlendProfileBoneEntry& Bone = Mask->GetEntry(Index);
                        FObject Entry = MakeShared<FJsonObject>();
                        Entry->SetStringField(TEXT("bone"), Bone.BoneReference.BoneName.ToString());
                        Entry->SetNumberField(TEXT("scale"), Bone.BlendScale);
                        Entries.Add(MakeShared<FJsonValueObject>(Entry));
                    }
                }
                Detail->SetArrayField(TEXT("entries"), Entries);
                MaskDetails.Add(MakeShared<FJsonValueObject>(Detail));
            }
            Settings->SetArrayField(TEXT("blend_masks"), Masks);
            Settings->SetArrayField(TEXT("blend_mask_details"), MaskDetails);
            Row->SetObjectField(TEXT("layered_bone_blend"), Settings);
        }
        return Row;
    }

    void RigPin(const URigVMPin* Pin, FValues& Pins)
    {
        if (!Pin) return;
        FObject Row = MakeShared<FJsonObject>();
        Row->SetStringField(TEXT("path"), Pin->GetPinPath());
        Row->SetStringField(TEXT("type"), Pin->GetCPPType());
        Row->SetNumberField(TEXT("direction"), static_cast<int32>(Pin->GetDirection()));
        Row->SetStringField(TEXT("default_value"), Pin->GetDefaultValue());
        FValues Links;
        for (const URigVMLink* Link : Pin->GetLinks())
        {
            FObject Item = MakeShared<FJsonObject>();
            Item->SetStringField(TEXT("source"), Link->GetSourcePinPath());
            Item->SetStringField(TEXT("target"), Link->GetTargetPinPath());
            Links.Add(MakeShared<FJsonValueObject>(Item));
        }
        Row->SetArrayField(TEXT("links"), Links);
        Pins.Add(MakeShared<FJsonValueObject>(Row));
        for (const URigVMPin* Child : Pin->GetSubPins()) RigPin(Child, Pins);
    }

    void RigGraph(URigVMGraph* Graph, FValues& Graphs, TSet<const URigVMGraph*>& Visited)
    {
        if (!Graph || Visited.Contains(Graph)) return;
        Visited.Add(Graph);
        FObject Row = MakeShared<FJsonObject>();
        Row->SetStringField(TEXT("path"), Graph->GetPathName());
        FValues Nodes;
        for (URigVMNode* Item : Graph->GetNodes())
        {
            FObject Entry = MakeShared<FJsonObject>();
            Entry->SetStringField(TEXT("path"), Item->GetPathName());
            Entry->SetStringField(TEXT("class"), Item->GetClass()->GetPathName());
            Entry->SetStringField(TEXT("title"), Item->GetNodeTitle());
            Entry->SetArrayField(TEXT("properties"), Properties(Item));
            FValues Pins;
            for (const URigVMPin* ItemPin : Item->GetPins()) RigPin(ItemPin, Pins);
            Entry->SetArrayField(TEXT("pins"), Pins);
            Nodes.Add(MakeShared<FJsonValueObject>(Entry));
            if (URigVMCollapseNode* Collapse = Cast<URigVMCollapseNode>(Item))
                RigGraph(Collapse->GetContainedGraph(), Graphs, Visited);
        }
        Row->SetArrayField(TEXT("nodes"), Nodes);
        Graphs.Add(MakeShared<FJsonValueObject>(Row));
    }

    FObject Blueprint(UBlueprint* Blueprint)
    {
        FObject Row = MakeShared<FJsonObject>();
        Row->SetStringField(TEXT("path"), Blueprint->GetPathName());
        Row->SetStringField(TEXT("parent_class"), GetPathNameSafe(Blueprint->ParentClass));
        Row->SetStringField(TEXT("generated_class"), GetPathNameSafe(Blueprint->GeneratedClass));
        Row->SetNumberField(TEXT("status"), static_cast<int32>(Blueprint->Status));
        Row->SetArrayField(TEXT("cdo_properties"), Properties(Blueprint->GeneratedClass ? Blueprint->GeneratedClass->GetDefaultObject() : nullptr));
        TArray<UEdGraph*> AllGraphs;
        Blueprint->GetAllGraphs(AllGraphs);
        FValues Graphs;
        for (UEdGraph* Graph : AllGraphs)
        {
            if (!Graph) continue;
            FObject Entry = MakeShared<FJsonObject>();
            Entry->SetStringField(TEXT("path"), Graph->GetPathName());
            Entry->SetStringField(TEXT("class"), Graph->GetClass()->GetPathName());
            FValues Nodes;
            for (UEdGraphNode* Item : Graph->Nodes)
                if (Item) Nodes.Add(MakeShared<FJsonValueObject>(Node(Item)));
            Entry->SetArrayField(TEXT("nodes"), Nodes);
            Graphs.Add(MakeShared<FJsonValueObject>(Entry));
        }
        Row->SetArrayField(TEXT("graphs"), Graphs);
        if (URigVMBlueprint* Rig = Cast<URigVMBlueprint>(Blueprint))
        {
            FValues Models;
            TSet<const URigVMGraph*> Visited;
            for (URigVMGraph* Model : Rig->GetAllModels()) RigGraph(Model, Models, Visited);
            RigGraph(Rig->GetLocalFunctionLibrary(), Models, Visited);
            Row->SetArrayField(TEXT("rigvm_models"), Models);
        }
        return Row;
    }

    FObject AimOffset(UBlendSpace* Blend)
    {
        FObject Row = MakeShared<FJsonObject>();
        Row->SetStringField(TEXT("path"), Blend->GetPathName());
        Row->SetStringField(TEXT("class"), Blend->GetClass()->GetPathName());
        FValues Axes;
        for (int32 Index = 0; Index < 3; ++Index)
        {
            const FBlendParameter& Axis = Blend->GetBlendParameter(Index);
            FObject Item = MakeShared<FJsonObject>();
            Item->SetNumberField(TEXT("index"), Index);
            Item->SetStringField(TEXT("name"), Axis.DisplayName);
            Item->SetNumberField(TEXT("min"), Axis.Min);
            Item->SetNumberField(TEXT("max"), Axis.Max);
            Item->SetNumberField(TEXT("grid_num"), Axis.GridNum);
            Item->SetBoolField(TEXT("wrap_input"), Axis.bWrapInput);
            Axes.Add(MakeShared<FJsonValueObject>(Item));
        }
        Row->SetArrayField(TEXT("axes"), Axes);
        FValues Samples;
        for (const FBlendSample& Sample : Blend->GetBlendSamples())
        {
            FObject Item = MakeShared<FJsonObject>();
            Item->SetStringField(TEXT("animation"), GetPathNameSafe(Sample.Animation.Get()));
            Item->SetStringField(TEXT("sample_value"), Sample.SampleValue.ToString());
            Item->SetNumberField(TEXT("rate_scale"), Sample.RateScale);
            if (const UAnimSequence* Sequence = Sample.Animation.Get())
            {
                Item->SetNumberField(TEXT("additive_anim_type"), static_cast<int32>(Sequence->AdditiveAnimType.GetValue()));
                Item->SetNumberField(TEXT("ref_pose_type"), static_cast<int32>(Sequence->RefPoseType.GetValue()));
                Item->SetNumberField(TEXT("ref_frame_index"), Sequence->RefFrameIndex);
                Item->SetStringField(TEXT("ref_pose_sequence"), GetPathNameSafe(Sequence->RefPoseSeq.Get()));
                Item->SetNumberField(TEXT("play_length"), Sequence->GetPlayLength());
            }
            Samples.Add(MakeShared<FJsonValueObject>(Item));
        }
        Row->SetArrayField(TEXT("samples"), Samples);
        return Row;
    }

    bool SaveStagedBlueprint(UBlueprint* Blueprint, FObject Result, TFunctionRef<bool()> WriteCheckpoint)
    {
        UPackage* Package = Blueprint->GetOutermost();
        const FString OriginalFilename = FPaths::ConvertRelativePathToFull(
            FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension()));
        const FString StagedFilename = FPaths::ConvertRelativePathToFull(FPaths::Combine(
            FPaths::ProjectSavedDir(), TEXT("CharacterAnimationAudit/Staged"), Blueprint->GetName() + TEXT(".uasset")));
        Result->SetObjectField(TEXT("after"), CharacterAnimationAudit::Blueprint(Blueprint));
        Result->SetStringField(TEXT("phase"), TEXT("compiled_pending_staged_save"));
        Result->SetBoolField(TEXT("saved"), false);
        Result->SetBoolField(TEXT("original_overwritten"), false);
        Result->SetStringField(TEXT("package_filename"), OriginalFilename);
        Result->SetStringField(TEXT("staged_filename"), StagedFilename);
        if (!WriteCheckpoint()) return false;
        Package->FullyLoad();
        FlushAsyncLoading();
        ResetLoaders(Package);
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(StagedFilename), true);
        FSavePackageArgs SaveArgs;
        SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
        SaveArgs.SaveFlags = SAVE_NoError;
        SaveArgs.Error = GWarn;
        SaveArgs.bSlowTask = false;
        const bool bSaved = UPackage::SavePackage(Package, Blueprint, *StagedFilename, SaveArgs);
        Result->SetBoolField(TEXT("saved"), bSaved);
        Result->SetBoolField(TEXT("saved_to_staging"), bSaved);
        Result->SetStringField(TEXT("phase"), bSaved ? TEXT("saved_to_staging") : TEXT("staged_save_failed"));
        return bSaved;
    }

    bool CompileForAudit(UBlueprint* Blueprint, FObject Result)
    {
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
        FCompilerResultsLog CompilerLog;
        FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::None, &CompilerLog);
        Result->SetNumberField(TEXT("compile_errors"), CompilerLog.NumErrors);
        Result->SetNumberField(TEXT("compile_warnings"), CompilerLog.NumWarnings);
        FValues Messages;
        for (const TSharedRef<FTokenizedMessage>& Message : CompilerLog.Messages)
            Messages.Add(MakeShared<FJsonValueString>(Message->ToText().ToString()));
        Result->SetArrayField(TEXT("compile_messages"), Messages);
        return CompilerLog.NumErrors == 0 && Blueprint->Status != BS_Error;
    }

    bool FixLayerGraph(const TMap<FString, UBlueprint*>& Loaded, FObject Result, TFunctionRef<bool()> WriteCheckpoint)
    {
        const auto Fail = [&Result](const FString& Message)
        {
            Result->SetStringField(TEXT("error"), Message);
            UE_LOG(LogTemp, Error, TEXT("CharacterAnimationAudit Layer: %s"), *Message);
            return false;
        };
        const FString Folder = TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/");
        UBlueprint* const* LayerPtr = Loaded.Find(Folder + TEXT("ABP_Helldiver_Layer"));
        UBlueprint* Layer = LayerPtr ? *LayerPtr : nullptr;
        if (!Layer || Layer->GetOutermost()->GetName() != Folder + TEXT("ABP_Helldiver_Layer"))
            return Fail(TEXT("The exact base layer blueprint must be loaded."));
        for (const TCHAR* Name : {TEXT("SetupDodgeStartAnim"), TEXT("UpdateDodgeStartAnim"),
            TEXT("SetupDodgeLoopAnim"), TEXT("UpdateDodgeLoopAnim"), TEXT("SetupDodgeEndAnim"), TEXT("UpdateDodgeEndAnim")})
        {
            const UFunction* Function = Layer->ParentClass->FindFunctionByName(FName(Name));
            if (!Function || Function->NumParms != 2 || !Function->HasMetaData(TEXT("BlueprintThreadSafe")))
                return Fail(FString::Printf(TEXT("Expected two-parameter thread-safe native callback %s."), Name));
        }
        FObjectPropertyBase* JumpStartProperty = FindFProperty<FObjectPropertyBase>(Layer->ParentClass, TEXT("_jumpStart"));
        FObjectPropertyBase* JumpLoopProperty = FindFProperty<FObjectPropertyBase>(Layer->ParentClass, TEXT("_jumpStartLoop"));
        FObjectPropertyBase* ApexProperty = FindFProperty<FObjectPropertyBase>(Layer->ParentClass, TEXT("_jumpApex"));
        if (!JumpStartProperty || !JumpLoopProperty || !ApexProperty)
            return Fail(TEXT("Native _jumpStart, _jumpStartLoop and _jumpApex object fields are required."));
        TArray<UEdGraph*> AllGraphs;
        Layer->GetAllGraphs(AllGraphs);
        TMap<FName, UEdGraph*> Selected;
        for (const TCHAR* Name : {TEXT("JumpStartLayer"), TEXT("JumpApexLayer"), TEXT("DodgeLoopLayer"),
            TEXT("DodgeEndLayer"), TEXT("DodgeStartLayer"), TEXT("JumpStartLoopLayer")})
        {
            TArray<UEdGraph*> Matches;
            for (UEdGraph* Graph : AllGraphs) if (Graph && Graph->GetFName() == FName(Name)) Matches.Add(Graph);
            if (Matches.Num() != 1) return Fail(FString::Printf(TEXT("Expected exactly one graph %s."), Name));
            Selected.Add(FName(Name), Matches[0]);
        }
        for (const TCHAR* Name : {TEXT("JumpStartLayer"), TEXT("JumpApexLayer"), TEXT("DodgeLoopLayer"), TEXT("DodgeEndLayer")})
        {
            UEdGraph* Graph = Selected.FindChecked(FName(Name));
            if (Graph->Nodes.Num() != 1 || !Cast<UAnimGraphNode_Root>(Graph->Nodes[0]))
                return Fail(FString::Printf(TEXT("Expected only an unconnected root in %s; no package saved."), Name));
            UEdGraphPin* RootInput = Graph->Nodes[0]->FindPin(TEXT("Result"), EGPD_Input);
            if (!RootInput || RootInput->LinkedTo.Num() != 0)
                return Fail(FString::Printf(TEXT("Expected unconnected Result pin in %s."), Name));
        }
        TMap<FName, UAnimGraphNode_SequencePlayer*> ExistingPlayers;
        for (const TCHAR* Name : {TEXT("DodgeStartLayer"), TEXT("JumpStartLoopLayer")})
        {
            UEdGraph* Graph = Selected.FindChecked(FName(Name));
            TArray<UAnimGraphNode_SequencePlayer*> Players;
            for (UEdGraphNode* Node : Graph->Nodes) if (auto* Player = Cast<UAnimGraphNode_SequencePlayer>(Node)) Players.Add(Player);
            if (Graph->Nodes.Num() != 2 || Players.Num() != 1 || !Players[0]->FindPin(TEXT("Sequence"), EGPD_Input))
                return Fail(FString::Printf(TEXT("Expected exactly one existing sequence player and root in %s."), Name));
            ExistingPlayers.Add(FName(Name), Players[0]);
        }
        struct FConcreteData { UBlueprint* Blueprint; UAnimSequence* Jump; UAnimSequence* Loop; FString Weapon; };
        TArray<FConcreteData> Concrete;
        for (const TCHAR* Weapon : {TEXT("Rifle"), TEXT("Pistol"), TEXT("Unarmed")})
        {
            const FString WeaponName(Weapon);
            UBlueprint* const* BP = Loaded.Find(Folder + TEXT("ABP_Helldiver_") + WeaponName);
            const FString Prefix = Folder + TEXT("Assets/") + WeaponName + TEXT("/Jump/Helldiver_MM_") + WeaponName;
            UAnimSequence* Jump = LoadObject<UAnimSequence>(nullptr, *(Prefix + TEXT("_Jump_Start")));
            UAnimSequence* Loop = LoadObject<UAnimSequence>(nullptr, *(Prefix + TEXT("_Jump_Start_Loop")));
            if (!BP || !*BP || !Jump || !Loop || !(*BP)->GeneratedClass)
                return Fail(FString::Printf(TEXT("Missing concrete blueprint or jump assets for %s."), Weapon));
            Concrete.Add({*BP, Jump, Loop, WeaponName});
        }
        Layer->Modify();
        for (auto& Pair : Selected) Pair.Value->Modify();
        const auto ExposeSequence = [](UAnimGraphNode_Base* Node)
        {
            for (int32 Index = 0; Index < Node->ShowPinForProperties.Num(); ++Index)
                if (Node->ShowPinForProperties[Index].PropertyName == TEXT("Sequence"))
                { Node->SetPinVisibility(true, Index); break; }
            return Node->FindPin(TEXT("Sequence"), EGPD_Input);
        };
        const auto ConnectGetter = [Layer, &ExposeSequence](UEdGraph* Graph, UAnimGraphNode_Base* Target, const FProperty* Property)
        {
            UEdGraphPin* Sequence = ExposeSequence(Target);
            if (!Sequence) return false;
            FGraphNodeCreator<UK2Node_VariableGet> Creator(*Graph);
            UK2Node_VariableGet* Getter = Creator.CreateNode(false);
            Getter->SetFromProperty(Property, true, Layer->ParentClass);
            Getter->NodePosX = Target->NodePosX - 300;
            Getter->NodePosY = Target->NodePosY;
            Creator.Finalize();
            UEdGraphPin* Value = Getter->FindPin(Property->GetFName(), EGPD_Output);
            return Value && Graph->GetSchema()->TryCreateConnection(Value, Sequence);
        };
        for (const TCHAR* Name : {TEXT("JumpStartLayer"), TEXT("JumpApexLayer"), TEXT("DodgeLoopLayer"), TEXT("DodgeEndLayer")})
        {
            UEdGraph* Graph = Selected.FindChecked(FName(Name));
            UEdGraphNode* Root = Graph->Nodes[0];
            UAnimGraphNode_Base* NewNode = nullptr;
            const bool bDodgeEnd = FName(Name) == TEXT("DodgeEndLayer");
            if (bDodgeEnd)
            {
                FGraphNodeCreator<UAnimGraphNode_SequenceEvaluator> Creator(*Graph);
                auto* Evaluator = Creator.CreateNode(false);
                Evaluator->Node.SetShouldLoop(false);
                Evaluator->AlwaysDynamicProperties.Add(TEXT("Sequence"));
                Evaluator->AlwaysDynamicProperties.Add(TEXT("ExplicitTime"));
                Evaluator->BecomeRelevantFunction.SetSelfMember(TEXT("SetupDodgeEndAnim"));
                Evaluator->UpdateFunction.SetSelfMember(TEXT("UpdateDodgeEndAnim"));
                Creator.Finalize();
                NewNode = Evaluator;
            }
            else
            {
                FGraphNodeCreator<UAnimGraphNode_SequencePlayer> Creator(*Graph);
                auto* Player = Creator.CreateNode(false);
                Player->AlwaysDynamicProperties.Add(TEXT("Sequence"));
                Player->Node.SetLoopAnimation(FName(Name) == TEXT("DodgeLoopLayer"));
                if (FName(Name) == TEXT("DodgeLoopLayer"))
                {
                    Player->AlwaysDynamicProperties.Add(TEXT("Sequence"));
                    Player->BecomeRelevantFunction.SetSelfMember(TEXT("SetupDodgeLoopAnim"));
                    Player->UpdateFunction.SetSelfMember(TEXT("UpdateDodgeLoopAnim"));
                }
                Creator.Finalize();
                NewNode = Player;
            }
            NewNode->NodePosX = Root->NodePosX - 320;
            NewNode->NodePosY = Root->NodePosY;
            NewNode->NodeComment = TEXT("CharacterAnimationAudit.FixLayerGraph");
            UEdGraphPin* Pose = NewNode->FindPin(TEXT("Pose"), EGPD_Output);
            UEdGraphPin* Input = Root->FindPin(TEXT("Result"), EGPD_Input);
            if (!Pose || !Input || !Graph->GetSchema()->TryCreateConnection(Pose, Input))
                return Fail(FString::Printf(TEXT("Could not connect pose output in %s."), Name));
            if (FName(Name) == TEXT("JumpStartLayer") && !ConnectGetter(Graph, NewNode, JumpStartProperty))
                return Fail(TEXT("Could not connect _jumpStart."));
            if (FName(Name) == TEXT("JumpApexLayer") && !ConnectGetter(Graph, NewNode, ApexProperty))
                return Fail(TEXT("Could not connect _jumpApex."));
        }
        UAnimGraphNode_SequencePlayer* DodgeStart = ExistingPlayers.FindChecked(TEXT("DodgeStartLayer"));
        DodgeStart->Modify();
        DodgeStart->RemoveBindings(TEXT("Sequence"));
        DodgeStart->AlwaysDynamicProperties.Add(TEXT("Sequence"));
        DodgeStart->BecomeRelevantFunction.SetSelfMember(TEXT("SetupDodgeStartAnim"));
        DodgeStart->UpdateFunction.SetSelfMember(TEXT("UpdateDodgeStartAnim"));
        DodgeStart->Node.SetLoopAnimation(false);
        UAnimGraphNode_SequencePlayer* JumpLoop = ExistingPlayers.FindChecked(TEXT("JumpStartLoopLayer"));
        JumpLoop->Modify();
        JumpLoop->RemoveBindings(TEXT("Sequence"));
        JumpLoop->AlwaysDynamicProperties.Add(TEXT("Sequence"));
        JumpLoop->Node.SetLoopAnimation(true);
        if (!ConnectGetter(JumpLoop->GetGraph(), JumpLoop, JumpLoopProperty)) return Fail(TEXT("Could not connect _jumpStartLoop."));
        FValues Packages;
        FObject LayerResult = MakeShared<FJsonObject>();
        LayerResult->SetStringField(TEXT("blueprint"), Layer->GetPathName());
        Packages.Add(MakeShared<FJsonValueObject>(LayerResult));
        Result->SetArrayField(TEXT("packages"), Packages);
        if (!CompileForAudit(Layer, LayerResult)) return Fail(TEXT("Base layer compilation failed; no base package saved."));
        for (auto& Pair : Selected)
        {
            UAnimGraphNode_Root* Root = nullptr;
            for (UEdGraphNode* Node : Pair.Value->Nodes) if (auto* Found = Cast<UAnimGraphNode_Root>(Node)) { if (Root) return Fail(TEXT("Duplicate root after compile.")); Root = Found; }
            if (!Root || !Root->FindPin(TEXT("Result"), EGPD_Input) || Root->FindPin(TEXT("Result"), EGPD_Input)->LinkedTo.Num() != 1)
                return Fail(TEXT("Layer output contract failed after compilation."));
            UAnimGraphNode_Base* Source = Cast<UAnimGraphNode_Base>(Root->FindPin(TEXT("Result"), EGPD_Input)->LinkedTo[0]->GetOwningNode());
            if (!Source) return Fail(TEXT("Layer output does not come from an animation node."));
            FName ExpectedField;
            if (Pair.Key == TEXT("JumpStartLayer")) ExpectedField = TEXT("_jumpStart");
            if (Pair.Key == TEXT("JumpApexLayer")) ExpectedField = TEXT("_jumpApex");
            if (Pair.Key == TEXT("JumpStartLoopLayer")) ExpectedField = TEXT("_jumpStartLoop");
            if (!ExpectedField.IsNone())
            {
                UEdGraphPin* Sequence = Source->FindPin(TEXT("Sequence"), EGPD_Input);
                UK2Node_VariableGet* Getter = Sequence && Sequence->LinkedTo.Num() == 1 ? Cast<UK2Node_VariableGet>(Sequence->LinkedTo[0]->GetOwningNode()) : nullptr;
                if (!Getter || Getter->VariableReference.GetMemberName() != ExpectedField || Source->HasBinding(TEXT("Sequence")))
                    return Fail(TEXT("A jump sequence getter contract failed after compilation."));
            }
            FString CallbackStem;
            if (Pair.Key == TEXT("DodgeStartLayer")) CallbackStem = TEXT("DodgeStartAnim");
            if (Pair.Key == TEXT("DodgeLoopLayer")) CallbackStem = TEXT("DodgeLoopAnim");
            if (Pair.Key == TEXT("DodgeEndLayer")) CallbackStem = TEXT("DodgeEndAnim");
            if (!CallbackStem.IsEmpty() &&
                (Source->BecomeRelevantFunction.GetMemberName() != FName(*(TEXT("Setup") + CallbackStem)) ||
                 Source->UpdateFunction.GetMemberName() != FName(*(TEXT("Update") + CallbackStem)) ||
                 !Source->AlwaysDynamicProperties.Contains(TEXT("Sequence")) || Source->HasBinding(TEXT("Sequence")) ||
                 (Pair.Key == TEXT("DodgeEndLayer") && !Source->AlwaysDynamicProperties.Contains(TEXT("ExplicitTime")))))
                return Fail(TEXT("A dodge callback or dynamic property contract failed after compilation."));
        }
        if (!SaveStagedBlueprint(Layer, LayerResult, WriteCheckpoint)) return Fail(TEXT("Could not stage the base layer."));
        for (const FConcreteData& Data : Concrete)
        {
            FObject Item = MakeShared<FJsonObject>();
            Item->SetStringField(TEXT("blueprint"), Data.Blueprint->GetPathName());
            Packages.Add(MakeShared<FJsonValueObject>(Item));
            Result->SetArrayField(TEXT("packages"), Packages);
            Data.Blueprint->Modify();
            UObject* CDO = Data.Blueprint->GeneratedClass->GetDefaultObject();
            CDO->Modify();
            JumpStartProperty->SetObjectPropertyValue_InContainer(CDO, Data.Jump);
            JumpLoopProperty->SetObjectPropertyValue_InContainer(CDO, Data.Loop);
            Item->SetStringField(TEXT("jump_start"), Data.Jump->GetPathName());
            Item->SetStringField(TEXT("jump_start_loop"), Data.Loop->GetPathName());
            if (Data.Weapon == TEXT("Rifle"))
            {
                FObject Check = MakeShared<FJsonObject>();
                UBlendSpace* Crouch = LoadObject<UBlendSpace>(nullptr, *(Folder + TEXT("AimOffset/AO_Rifle_Crouch")));
                FObjectPropertyBase* IdleProperty = FindFProperty<FObjectPropertyBase>(CDO->GetClass(), TEXT("_crouchIdleAnim"));
                FObjectPropertyBase* AOProperty = FindFProperty<FObjectPropertyBase>(CDO->GetClass(), TEXT("_aoCrouch"));
                UObject* Idle = IdleProperty ? IdleProperty->GetObjectPropertyValue_InContainer(CDO) : nullptr;
                bool bSameReference = Crouch && Idle && AOProperty && Crouch->GetBlendSamples().Num() > 0;
                if (Crouch) for (const FBlendSample& Sample : Crouch->GetBlendSamples())
                    bSameReference &= Sample.Animation && Sample.Animation->RefPoseSeq.Get() == Idle && Sample.Animation->AdditiveAnimType == AAT_RotationOffsetMeshSpace;
                Check->SetStringField(TEXT("candidate"), GetPathNameSafe(Crouch));
                Check->SetStringField(TEXT("base_idle"), GetPathNameSafe(Idle));
                Check->SetBoolField(TEXT("all_sample_references_match"), bSameReference);
                Check->SetBoolField(TEXT("changed"), bSameReference);
                if (Crouch) Check->SetObjectField(TEXT("candidate_audit"), AimOffset(Crouch));
                if (bSameReference) AOProperty->SetObjectPropertyValue_InContainer(CDO, Crouch);
                Item->SetObjectField(TEXT("crouch_ao_reference_check"), Check);
            }
            if (!CompileForAudit(Data.Blueprint, Item)) return Fail(TEXT("Concrete layer compilation failed; its package was not saved."));
            CDO = Data.Blueprint->GeneratedClass->GetDefaultObject();
            if (JumpStartProperty->GetObjectPropertyValue_InContainer(CDO) != Data.Jump || JumpLoopProperty->GetObjectPropertyValue_InContainer(CDO) != Data.Loop)
                return Fail(TEXT("Concrete jump defaults changed during compilation; package not saved."));
            if (!SaveStagedBlueprint(Data.Blueprint, Item, WriteCheckpoint)) return Fail(TEXT("Could not stage a concrete layer."));
        }
        return true;
    }

    bool FixLeftHandIK(UBlueprint* Blueprint, FObject Result, bool bFixAimGraph, TFunctionRef<bool()> WriteCheckpoint)
    {
        const auto Fail = [&Result](const FString& Message)
        {
            Result->SetBoolField(TEXT("saved"), false);
            Result->SetStringField(TEXT("error"), Message);
            UE_LOG(LogTemp, Error, TEXT("CharacterAnimationAudit: %s"), *Message);
            return false;
        };
        if (!Blueprint || Blueprint->GetOutermost()->GetName() != MainBlueprint)
            return Fail(TEXT("Only the exact MK5 blueprint is eligible for this patch."));
        FStructProperty* TargetProperty = FindFProperty<FStructProperty>(Blueprint->ParentClass, TEXT("_leftHandIKTargetWorld"));
        FFloatProperty* AlphaProperty = FindFProperty<FFloatProperty>(Blueprint->ParentClass, TEXT("_leftHandIKAlpha"));
        if (!TargetProperty || TargetProperty->Struct != TBaseStructure<FVector>::Get() || !AlphaProperty)
            return Fail(TEXT("Native FVector _leftHandIKTargetWorld / float _leftHandIKAlpha properties are required."));
        FFloatProperty* RigAlphaProperty = bFixAimGraph ? FindFProperty<FFloatProperty>(Blueprint->ParentClass, TEXT("_aimRigAlpha")) : nullptr;
        if (bFixAimGraph && !RigAlphaProperty)
            return Fail(TEXT("FixAimGraph requires native float _aimRigAlpha."));
        TArray<UEdGraph*> Graphs;
        Blueprint->GetAllGraphs(Graphs);
        TArray<UAnimGraphNode_TwoBoneIK*> Candidates;
        TArray<UAnimGraphNode_ControlRig*> RigCandidates;
        for (UEdGraph* Graph : Graphs)
        {
            if (!Graph) continue;
            for (UEdGraphNode* Item : Graph->Nodes)
            {
                if (UAnimGraphNode_TwoBoneIK* IK = Cast<UAnimGraphNode_TwoBoneIK>(Item))
                    if (IK->Node.IKBone.BoneName == TEXT("hand_l")) Candidates.Add(IK);
                if (UAnimGraphNode_ControlRig* Rig = Cast<UAnimGraphNode_ControlRig>(Item)) RigCandidates.Add(Rig);
            }
        }
        if (Candidates.Num() != 1)
            return Fail(FString::Printf(TEXT("Expected exactly one hand_l TwoBoneIK node; found %d."), Candidates.Num()));
        UAnimGraphNode_TwoBoneIK* IK = Candidates[0];
        UEdGraph* Graph = IK->GetGraph();
        const UEdGraphSchema* Schema = Graph ? Graph->GetSchema() : nullptr;
        UEdGraphPin* TargetPin = IK->FindPin(TEXT("EffectorLocation"), EGPD_Input);
        UEdGraphPin* AlphaPin = IK->FindPin(TEXT("Alpha"), EGPD_Input);
        if (!Schema || !TargetPin || !AlphaPin || IK->Node.EffectorLocationSpace != BCS_WorldSpace ||
            IK->Node.AlphaInputType != EAnimAlphaInputType::Float)
            return Fail(TEXT("Expected existing WorldSpace EffectorLocation and float Alpha input pins."));
        UAnimGraphNode_ControlRig* Rig = nullptr;
        UEdGraphPin* RigAlphaPin = nullptr;
        if (bFixAimGraph)
        {
            if (RigCandidates.Num() != 1)
                return Fail(FString::Printf(TEXT("Expected exactly one ControlRig node; found %d."), RigCandidates.Num()));
            Rig = RigCandidates[0];
            const FString ExpectedRigClass = TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/ControlRIg/CR_Helldiver_Locomotion_Aiming.CR_Helldiver_Locomotion_Aiming_C");
            RigAlphaPin = Rig->FindPin(TEXT("Alpha"), EGPD_Input);
            const FEnumProperty* RigAlphaType = FindFProperty<FEnumProperty>(FAnimNode_ControlRig::StaticStruct(), TEXT("AlphaInputType"));
            const bool bFloatRigAlpha = RigAlphaType &&
                RigAlphaType->GetUnderlyingProperty()->GetSignedIntPropertyValue(
                    RigAlphaType->ContainerPtrToValuePtr<void>(&Rig->Node)) == static_cast<int64>(EAnimAlphaInputType::Float);
            if (Rig->GetGraph() != Graph || GetPathNameSafe(Rig->Node.GetControlRigClass().Get()) != ExpectedRigClass || !RigAlphaPin || !bFloatRigAlpha)
                return Fail(TEXT("Expected the exact aiming ControlRig in the same graph with a float Alpha pin."));
        }
        const auto IsLinkedGetter = [](const UEdGraphPin* Pin, FName Name)
        {
            return Pin->LinkedTo.Num() == 1 && Cast<UK2Node_VariableGet>(Pin->LinkedTo[0]->GetOwningNode()) &&
                CastChecked<UK2Node_VariableGet>(Pin->LinkedTo[0]->GetOwningNode())->VariableReference.GetMemberName() == Name;
        };
        if (IsLinkedGetter(TargetPin, TargetProperty->GetFName()) && IsLinkedGetter(AlphaPin, AlphaProperty->GetFName()) &&
            (!bFixAimGraph || IsLinkedGetter(RigAlphaPin, RigAlphaProperty->GetFName())))
        {
            Result->SetBoolField(TEXT("already_patched"), true);
            Result->SetBoolField(TEXT("saved"), false);
            return true;
        }
        Blueprint->Modify(); Graph->Modify(); IK->Modify();
        if (Rig) Rig->Modify();
        const auto Getter = [Graph, Blueprint, IK](const FProperty* Property, int32 OffsetY)
        {
            FGraphNodeCreator<UK2Node_VariableGet> Creator(*Graph);
            UK2Node_VariableGet* NewNode = Creator.CreateNode(false);
            NewNode->SetFromProperty(Property, true, Blueprint->ParentClass);
            NewNode->NodePosX = IK->NodePosX - 320;
            NewNode->NodePosY = IK->NodePosY + OffsetY;
            NewNode->SetFlags(RF_Transactional);
            Creator.Finalize();
            return NewNode;
        };
        UK2Node_VariableGet* TargetGetter = Getter(TargetProperty, -100);
        UK2Node_VariableGet* AlphaGetter = Getter(AlphaProperty, 60);
        UEdGraphPin* TargetOut = TargetGetter->FindPin(TargetProperty->GetFName(), EGPD_Output);
        UEdGraphPin* AlphaOut = AlphaGetter->FindPin(AlphaProperty->GetFName(), EGPD_Output);
        if (!TargetOut || !AlphaOut) return Fail(TEXT("Could not allocate native getter pins. No package saved."));
        UEdGraphPin* RigAlphaOut = nullptr;
        if (bFixAimGraph)
        {
            UK2Node_VariableGet* RigAlphaGetter = Getter(RigAlphaProperty, 220);
            RigAlphaOut = RigAlphaGetter->FindPin(RigAlphaProperty->GetFName(), EGPD_Output);
            if (!RigAlphaOut) return Fail(TEXT("Could not allocate native rig alpha getter pin. No package saved."));
        }
        TArray<UEdGraphNode*> DetachedSources;
        for (const UEdGraphPin* Linked : TargetPin->LinkedTo) DetachedSources.AddUnique(Linked->GetOwningNode());
        for (const UEdGraphPin* Linked : AlphaPin->LinkedTo) DetachedSources.AddUnique(Linked->GetOwningNode());
        if (RigAlphaPin)
            for (const UEdGraphPin* Linked : RigAlphaPin->LinkedTo) DetachedSources.AddUnique(Linked->GetOwningNode());
        Schema->BreakPinLinks(*TargetPin, true);
        Schema->BreakPinLinks(*AlphaPin, true);
        if (!Schema->TryCreateConnection(TargetOut, TargetPin) || !Schema->TryCreateConnection(AlphaOut, AlphaPin))
            return Fail(TEXT("Schema rejected a native getter connection. No package saved."));
        if (RigAlphaPin)
        {
            Schema->BreakPinLinks(*RigAlphaPin, true);
            if (!Schema->TryCreateConnection(RigAlphaOut, RigAlphaPin))
                return Fail(TEXT("Schema rejected the native aiming rig alpha connection. No package saved."));
        }
        // Remove only orphaned pure data nodes upstream of these two replaced inputs.
        // Shared producers stay connected and are never removed.
        FValues RemovedSources;
        for (int32 Index = 0; Index < DetachedSources.Num(); ++Index)
        {
            UEdGraphNode* Source = DetachedSources[Index];
            const UK2Node* K2Source = Cast<UK2Node>(Source);
            if (!Source || Source->GetGraph() != Graph || !K2Source || !K2Source->IsNodePure() ||
                !Graph->Nodes.Contains(Source)) continue;
            bool bHasConsumer = false;
            for (const UEdGraphPin* SourcePin : Source->Pins)
                if (SourcePin && SourcePin->Direction == EGPD_Output && SourcePin->LinkedTo.Num() > 0)
                    bHasConsumer = true;
            if (bHasConsumer) continue;
            for (const UEdGraphPin* SourcePin : Source->Pins)
                if (SourcePin && SourcePin->Direction == EGPD_Input)
                    for (const UEdGraphPin* Linked : SourcePin->LinkedTo)
                        DetachedSources.Add(Linked->GetOwningNode());
            RemovedSources.Add(MakeShared<FJsonValueString>(Source->GetPathName()));
            FBlueprintEditorUtils::RemoveNode(Blueprint, Source, true);
        }
        Result->SetArrayField(TEXT("removed_orphaned_input_nodes"), RemovedSources);
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
        FCompilerResultsLog CompilerLog;
        FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::None, &CompilerLog);
        Result->SetNumberField(TEXT("compile_errors"), CompilerLog.NumErrors);
        Result->SetNumberField(TEXT("compile_warnings"), CompilerLog.NumWarnings);
        FValues Messages;
        for (const TSharedRef<FTokenizedMessage>& Message : CompilerLog.Messages)
            Messages.Add(MakeShared<FJsonValueString>(Message->ToText().ToString()));
        Result->SetArrayField(TEXT("compile_messages"), Messages);
        if (CompilerLog.NumErrors != 0 || Blueprint->Status == BS_Error)
            return Fail(TEXT("Blueprint compilation failed. No package saved."));
        // Compilation may reconstruct pins, so find them again for the final contract check.
        TargetPin = IK->FindPin(TEXT("EffectorLocation"), EGPD_Input);
        AlphaPin = IK->FindPin(TEXT("Alpha"), EGPD_Input);
        if (!TargetPin || !AlphaPin || !IsLinkedGetter(TargetPin, TargetProperty->GetFName()) || !IsLinkedGetter(AlphaPin, AlphaProperty->GetFName()))
            return Fail(TEXT("Compiled graph does not match the requested getter contract. No package saved."));
        if (bFixAimGraph)
        {
            RigAlphaPin = Rig->FindPin(TEXT("Alpha"), EGPD_Input);
            if (!RigAlphaPin || !IsLinkedGetter(RigAlphaPin, RigAlphaProperty->GetFName()))
                return Fail(TEXT("Compiled rig Alpha does not match the native aiming contract. No package saved."));
            Result->SetBoolField(TEXT("aim_rig_alpha_patched"), true);
        }
        return SaveStagedBlueprint(Blueprint, Result, WriteCheckpoint) || Fail(TEXT("Staged SavePackage failed."));
    }
}

UCharacterAnimationAuditCommandlet::UCharacterAnimationAuditCommandlet()
{
    IsClient = false;
    IsServer = false;
    IsEditor = true;
    LogToConsole = true;
}

int32 UCharacterAnimationAuditCommandlet::Main(const FString& Params)
{
    using namespace CharacterAnimationAudit;
    FString ReportPath;
    if (!FParse::Value(*Params, TEXT("Report="), ReportPath))
        ReportPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("CharacterAnimationAudit/report.json"));
    ReportPath = FPaths::ConvertRelativePathToFull(ReportPath);
    const bool bFixAimGraph = FParse::Param(*Params, TEXT("FixAimGraph"));
    const bool bFix = bFixAimGraph || FParse::Param(*Params, TEXT("FixLeftHandIK"));
    const bool bFixLayerGraph = FParse::Param(*Params, TEXT("FixLayerGraph"));
    const bool bVerifyPlayback = FParse::Param(*Params, TEXT("VerifyPlayback"));
    const bool bVerifyProneAnchor = FParse::Param(*Params, TEXT("VerifyProneAnchor"));
    const bool bVerifyProneRoll = FParse::Param(*Params, TEXT("VerifyProneRoll"));
    const bool bFixProneAnchor = FParse::Param(*Params, TEXT("FixProneAnchor"));
    const bool bRemoveProneAnchor = FParse::Param(*Params, TEXT("RemoveProneAnchor"));
    const bool bAuditProneAssets = FParse::Param(*Params, TEXT("AuditProneAssets"));
    const bool bBuildProneCanonical = FParse::Param(*Params, TEXT("BuildProneCanonical"));
    const bool bBuildProneUnified = FParse::Param(*Params, TEXT("BuildProneUnified"));
    FObject Report = MakeShared<FJsonObject>();
    Report->SetStringField(TEXT("mode"), bRemoveProneAnchor ? TEXT("RemoveProneAnchor") : bFixProneAnchor ? TEXT("RetiredFixProneAnchor") : bFixLayerGraph ? (bFix ? TEXT("FixAimAndLayerGraph") : TEXT("FixLayerGraph")) :
        bFixAimGraph ? TEXT("FixAimGraph") : bFix ? TEXT("FixLeftHandIK") : bVerifyProneRoll ? TEXT("VerifyProneRoll") : bVerifyProneAnchor ? TEXT("RetiredVerifyProneAnchor") : bVerifyPlayback ? TEXT("VerifyPlayback") : TEXT("AuditOnly"));
    Report->SetBoolField(TEXT("verify_playback_requested"), bVerifyPlayback);
    Report->SetBoolField(TEXT("verify_prone_anchor_requested"), bVerifyProneAnchor);
    Report->SetBoolField(TEXT("verify_prone_roll_requested"), bVerifyProneRoll);
    Report->SetBoolField(TEXT("fix_prone_anchor_requested"), bFixProneAnchor);
    Report->SetBoolField(TEXT("remove_prone_anchor_requested"), bRemoveProneAnchor);
    Report->SetStringField(TEXT("engine_version"), FEngineVersion::Current().ToString());
    if (bFixProneAnchor || bVerifyProneAnchor)
    {
        Report->SetStringField(TEXT("error"), TEXT("FixProneAnchor and VerifyProneAnchor are retired: head pinning does not implement the intended prone roll. Use RemoveProneAnchor to stage removal and VerifyProneRoll for the current behavior."));
        Report->SetNumberField(TEXT("failures"), 1);
        Report->SetStringField(TEXT("phase"), TEXT("retired_switch_rejected_no_assets_changed"));
        WriteReport(Report, ReportPath);
        UE_LOG(LogTemp, Error, TEXT("Head anchor insertion and verification are retired; no asset changed. Use -RemoveProneAnchor or -VerifyProneRoll."));
        return 1;
    }
    FValues Assets;
    UBlueprint* MainBP = nullptr;
    TMap<FString, UBlueprint*> Loaded;
    int32 Failures = 0;
    const TArray<FString> Paths = {
        MainBlueprint,
        TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/ABP_Helldiver_Layer"),
        TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/ABP_Helldiver_Rifle"),
        TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/ABP_Helldiver_Pistol"),
        TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/ABP_Helldiver_Unarmed"),
        TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/ControlRIg/CR_Helldiver_Locomotion_Aiming")
    };
    for (const FString& Path : Paths)
    {
        UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *Path);
        if (BP)
        {
            Loaded.Add(Path, BP);
            Assets.Add(MakeShared<FJsonValueObject>(CharacterAnimationAudit::Blueprint(BP)));
            if (Path == MainBlueprint) MainBP = BP;
        }
        else
        {
            FObject Failure = MakeShared<FJsonObject>();
            Failure->SetStringField(TEXT("path"), Path);
            Failure->SetStringField(TEXT("error"), TEXT("Blueprint could not be loaded."));
            Assets.Add(MakeShared<FJsonValueObject>(Failure));
            ++Failures;
        }
    }
    Report->SetArrayField(TEXT("assets_before"), Assets);
    FValues AimOffsets;
    for (const TCHAR* Name : { TEXT("AO_ProneFwd"), TEXT("AO_ProneBwd"), TEXT("AO_ProneLeft"), TEXT("AO_ProneRight") })
    {
        const FString Path = FString(TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/AimOffset/")) + Name;
        if (UBlendSpace* Blend = LoadObject<UBlendSpace>(nullptr, *Path))
            AimOffsets.Add(MakeShared<FJsonValueObject>(AimOffset(Blend)));
        else
        {
            FObject Failure = MakeShared<FJsonObject>();
            Failure->SetStringField(TEXT("path"), Path);
            Failure->SetStringField(TEXT("error"), TEXT("AimOffset could not be loaded."));
            AimOffsets.Add(MakeShared<FJsonValueObject>(Failure));
            ++Failures;
        }
    }
    Report->SetArrayField(TEXT("aim_offsets"), AimOffsets);
    if (bAuditProneAssets)
    {
        FObject Geometry = MakeShared<FJsonObject>();
        Report->SetObjectField(TEXT("prone_asset_geometry"), Geometry);
        if (!AuditProneAssetGeometry(Geometry)) ++Failures;
    }
    Report->SetNumberField(TEXT("failures"), Failures);
    Report->SetStringField(TEXT("phase"), TEXT("audit_complete_before_patch"));
    const auto WriteCheckpoint = [&Report, &ReportPath]() { return WriteReport(Report, ReportPath); };
    if (!WriteCheckpoint())
    {
        UE_LOG(LogTemp, Error, TEXT("Could not write initial audit report %s; no graph patched."), *ReportPath);
        return 1;
    }
    if (bFix)
    {
        FObject Patch = MakeShared<FJsonObject>();
        Report->SetObjectField(TEXT("patch"), Patch);
        Report->SetStringField(TEXT("phase"), TEXT("patch_in_progress"));
        if (!FixLeftHandIK(MainBP, Patch, bFixAimGraph, WriteCheckpoint)) ++Failures;
    }
    if (bFixLayerGraph)
    {
        FObject LayerPatch = MakeShared<FJsonObject>();
        Report->SetObjectField(TEXT("layer_patch"), LayerPatch);
        Report->SetStringField(TEXT("phase"), TEXT("layer_patch_in_progress"));
        if (!FixLayerGraph(Loaded, LayerPatch, WriteCheckpoint)) ++Failures;
    }
    if (bRemoveProneAnchor)
    {
        FObject AnchorPatch = MakeShared<FJsonObject>();
        Report->SetObjectField(TEXT("prone_anchor_patch"), AnchorPatch);
        Report->SetStringField(TEXT("phase"), TEXT("prone_anchor_removal_in_progress"));
        if (!RemoveProneHeadAnchor(MainBP, AnchorPatch, WriteCheckpoint)) ++Failures;
    }
    if (bBuildProneCanonical)
    {
        FObject Build = MakeShared<FJsonObject>();
        Report->SetObjectField(TEXT("prone_canonical_assets"), Build);
        Report->SetStringField(TEXT("phase"), TEXT("prone_canonical_build_in_progress"));
        TMap<FString, UAnimSequence*> SourceIdle, CanonicalIdle;
        TMap<FString, UBlendSpace*> CanonicalAO;
        UBlueprint* Rifle = Loaded.FindRef(TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/ABP_Helldiver_Rifle"));
        UObject* CDO = Rifle && Rifle->GeneratedClass ? Rifle->GeneratedClass->GetDefaultObject() : nullptr;
        FStructProperty* Idle = CDO ? FindFProperty<FStructProperty>(CDO->GetClass(), TEXT("_proneIdleAnims")) : nullptr;
        if (Idle) for (const TCHAR* Direction : { TEXT("Forward"), TEXT("Right"), TEXT("Backward"), TEXT("Left") })
            if (FObjectPropertyBase* Field = FindFProperty<FObjectPropertyBase>(Idle->Struct, FName(Direction)))
                SourceIdle.Add(Direction, Cast<UAnimSequence>(Field->GetObjectPropertyValue_InContainer(Idle->ContainerPtrToValuePtr<void>(CDO))));
        if (SourceIdle.Num() != 4 || !BuildProneCanonicalAssets(SourceIdle, CanonicalIdle, CanonicalAO, Build)) ++Failures;
        else
        {
            FObject Defaults = MakeShared<FJsonObject>();
            Report->SetObjectField(TEXT("prone_canonical_defaults"), Defaults);
            if (!StageCanonicalProneDefaults(Loaded, CanonicalIdle, CanonicalAO, Defaults, WriteCheckpoint)) ++Failures;
        }
        WriteCheckpoint();
    }
    if (bBuildProneUnified)
    {
        FObject Build = MakeShared<FJsonObject>();
        Report->SetObjectField(TEXT("prone_unified_assets"), Build);
        Report->SetStringField(TEXT("phase"), TEXT("prone_unified_build_in_progress"));
        TMap<FString, UAnimSequence*> Idle;
        TMap<FString, UBlendSpace*> AO;
        if (!BuildUnifiedProneAssets(Idle, AO, Build)) ++Failures;
        else
        {
            FObject Defaults = MakeShared<FJsonObject>();
            Report->SetObjectField(TEXT("prone_unified_defaults"), Defaults);
            if (!StageUnifiedProneDefaults(Loaded, Idle.FindRef(TEXT("Forward")), AO.FindRef(TEXT("Forward")), Defaults, WriteCheckpoint)) ++Failures;
        }
        WriteCheckpoint();
    }
    if (bVerifyPlayback)
    {
        FObject Playback = MakeShared<FJsonObject>();
        Report->SetObjectField(TEXT("playback_verification"), Playback);
        Report->SetStringField(TEXT("phase"), TEXT("playback_verification_in_progress"));
        WriteCheckpoint();
        if (!VerifyCharacterAnimationPlayback(Playback)) ++Failures;
    }
    if (bVerifyProneRoll)
    {
        FObject Roll = MakeShared<FJsonObject>();
        Report->SetObjectField(TEXT("prone_roll_verification"), Roll);
        Report->SetStringField(TEXT("phase"), TEXT("prone_roll_verification_in_progress"));
        WriteCheckpoint();
        if (!VerifyCharacterProneRoll(Roll)) ++Failures;
    }
    Report->SetNumberField(TEXT("failures"), Failures);
    Report->SetStringField(TEXT("phase"), TEXT("complete"));
    if (!WriteCheckpoint())
    {
        UE_LOG(LogTemp, Error, TEXT("Could not write audit report %s"), *ReportPath);
        return 1;
    }
    UE_LOG(LogTemp, Display, TEXT("CharacterAnimationAudit wrote %s; failures=%d"), *ReportPath, Failures);
    return Failures == 0 ? 0 : 1;
}
