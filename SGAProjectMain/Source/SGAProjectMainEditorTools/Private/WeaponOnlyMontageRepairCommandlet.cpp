#include "WeaponOnlyMontageRepairCommandlet.h"
#include "AnimStateTransitionNode.h"
#include "AnimStateConduitNode.h"
#include "AnimStateAliasNode.h"
#include "AnimStateNode.h"
#include "AnimGraphNode_SequencePlayer.h"
#include "AnimGraphNode_TransitionResult.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "K2Node_VariableGet.h"
#include "K2Node_CallFunction.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/CompilerResultsLog.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UObject/SavePackage.h"
#include "UObject/UnrealType.h"
#include "../../SGAProjectMain/Character/CharacterAnimLayerInstance.h"

UWeaponOnlyMontageRepairCommandlet::UWeaponOnlyMontageRepairCommandlet()
{ IsClient=false; IsServer=false; IsEditor=true; LogToConsole=true; }

namespace
{
    UEdGraphNode* Find(UBlueprint* BP,const TCHAR* Guid)
    {
        FGuid Id; FGuid::Parse(Guid,Id); TArray<UEdGraph*> Graphs; BP->GetAllGraphs(Graphs);
        for (auto* G:Graphs) for(UEdGraphNode* N:G->Nodes) if(N->NodeGuid==Id) return N;
        return nullptr;
    }
    bool Rule(UEdGraph* G,UBlueprint* BP,const TCHAR* Variable,bool Constant=false,bool Invert=false)
    {
        if(!G || FBlueprintEditorUtils::FindBlueprintForGraph(G)!=BP) return false;
        UAnimGraphNode_TransitionResult* Result=nullptr;
        for(UEdGraphNode* N:G->Nodes) if(auto* R=Cast<UAnimGraphNode_TransitionResult>(N)) Result=R;
        if(!Result) return false;
        auto* Input=Result->FindPin(TEXT("bCanEnterTransition")); if(!Input) return false;
        Input->BreakAllPinLinks();
        if(!Variable)
        {
            FGraphNodeCreator<UK2Node_CallFunction> C(*G); auto* Literal=C.CreateNode(false);
            Literal->SetFromFunction(UKismetSystemLibrary::StaticClass()->FindFunctionByName(TEXT("MakeLiteralBool"))); C.Finalize();
            Literal->FindPin(TEXT("Value"))->DefaultValue=Constant?TEXT("true"):TEXT("false");
            return G->GetSchema()->TryCreateConnection(Literal->GetReturnValuePin(),Input);
        }
        auto* Property=FindFProperty<FBoolProperty>(BP->ParentClass,Variable); if(!Property) return false;
        FGraphNodeCreator<UK2Node_VariableGet> C(*G); auto* Getter=C.CreateNode(false);
        Getter->SetFromProperty(Property,true,BP->ParentClass); C.Finalize();
        UEdGraphPin* Value=Getter->GetValuePin();
        if(Invert)
        {
            FGraphNodeCreator<UK2Node_CallFunction> F(*G); auto* Not=F.CreateNode(false);
            Not->SetFromFunction(UKismetMathLibrary::StaticClass()->FindFunctionByName(TEXT("Not_PreBool"))); F.Finalize();
            if(!G->GetSchema()->TryCreateConnection(Value,Not->FindPin(TEXT("A")))) return false;
            Value=Not->GetReturnValuePin();
        }
        return G->GetSchema()->TryCreateConnection(Value,Input);
    }
    bool Transition(UBlueprint* BP,const TCHAR* Guid,const TCHAR* Variable,bool Invert=false)
    {
        auto* T=Cast<UAnimStateTransitionNode>(Find(BP,Guid)); if(!T || T->IsBoundGraphShared()) return false;
        T->bAutomaticRuleBasedOnSequencePlayerInState=false;
        return Rule(T->BoundGraph,BP,Variable,false,Invert);
    }
}
int32 UWeaponOnlyMontageRepairCommandlet::Main(const FString& Params)
{
    const FString Prefix=TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/");
    auto* Main=LoadObject<UAnimBlueprint>(nullptr,*(Prefix+TEXT("ABP_Helldiver_MK5.ABP_Helldiver_MK5")));
    auto* Layer=LoadObject<UAnimBlueprint>(nullptr,*(Prefix+TEXT("ABP_Helldiver_Layer.ABP_Helldiver_Layer")));
    if(!Main || !Layer) return 1;
    if(!Transition(Main,TEXT("73CC8C8F43F545CA4905FA8482F666A8"),TEXT("_stanceTransitionRequested")) ||
       !Transition(Main,TEXT("02AB5C484AE323667C4F50A23548B5BF"),TEXT("_stanceTransitionComplete")) ||
       !Transition(Main,TEXT("551013424B3C233F9E8E3CA535EE8272"),TEXT("_stanceTransitionMoving"),true) ||
       !Transition(Main,TEXT("7BCE30EC4F44450157121682B05FA7FA"),TEXT("_stanceTransitionMoving"))) return 2;
    auto* End=Cast<UAnimStateConduitNode>(Find(Main,TEXT("F2EF51B04827F471443341805D309649")));
    if(!End || !Rule(End->BoundGraph,Main,nullptr,true)) return 3;
    auto* Idle=Cast<UAnimStateNode>(Find(Main,TEXT("EA35DCF3402109CFEDF78EAC92012504")));
    auto* Cycle=Cast<UAnimStateNode>(Find(Main,TEXT("34B914FE439BA0596BEE8782AB311DC0")));
    auto* ToIdle=Cast<UAnimStateTransitionNode>(Find(Main,TEXT("551013424B3C233F9E8E3CA535EE8272")));
    auto* ToCycle=Cast<UAnimStateTransitionNode>(Find(Main,TEXT("7BCE30EC4F44450157121682B05FA7FA")));
    if(!Idle || !Cycle || !ToIdle || !ToCycle) return 3;
    // The existing CycleAlias resolves to Start. Only the new stance return
    // transitions are relinked; jump/dodge aliases keep their existing behavior.
    ToIdle->GetInputPin()->BreakAllPinLinks(false);
    ToIdle->GetOutputPin()->BreakAllPinLinks(false);
    ToCycle->GetInputPin()->BreakAllPinLinks(false);
    ToCycle->GetOutputPin()->BreakAllPinLinks(false);
    ToIdle->CreateConnections(End,Idle);
    ToCycle->CreateConnections(End,Cycle);
    auto* Stance=Cast<UAnimStateNode>(Find(Main,TEXT("F501946F4739ED2856BFC9AA2BEFAB4D")));
    if(!Stance) return 3;
    for(UEdGraphPin* Pin:Stance->Pins)
    {
        const auto Links=Pin->LinkedTo;
        for(UEdGraphPin* Link:Links)
            if(!Cast<UAnimStateTransitionNode>(Link->GetOwningNode())) Pin->BreakLinkTo(Link);
    }
    auto* Alias=Cast<UAnimStateAliasNode>(Find(Main,TEXT("CF6E063146D7CAEF6117B28CA5D31589")));
    if(!Alias) return 4;
    Alias->bGlobalAlias=false; Alias->GetAliasedStates().Empty();
    for(UEdGraphNode* N:Alias->GetGraph()->Nodes)
        if(auto* S=Cast<UAnimStateNode>(N))
            if(S->GetStateName()==TEXT("Idle") || S->GetStateName()==TEXT("Start") || S->GetStateName()==TEXT("Cycle") || S->GetStateName()==TEXT("Stop") || S->GetStateName()==TEXT("Pivot"))
                Alias->GetAliasedStates().Add(S);
    auto* Enter=Cast<UAnimStateTransitionNode>(Find(Main,TEXT("73CC8C8F43F545CA4905FA8482F666A8")));
    for(UEdGraphNode* N:Alias->GetGraph()->Nodes) if(auto* T=Cast<UAnimStateTransitionNode>(N))
        if(T!=Enter && T->PriorityOrder<=0) T->PriorityOrder=1;
    Enter->PriorityOrder=0;
    // Keep the old state graph intact. Disabling its entry avoids the shared-graph deletion crash.
    auto* Old=Cast<UAnimStateTransitionNode>(Find(Layer,TEXT("C49EC5044016DDC67949E1A0A7F3895F")));
    if(!Old || !Rule(Old->BoundGraph,Layer,nullptr,false)) return 5;
    auto* Player=Cast<UAnimGraphNode_SequencePlayer>(Find(Layer,TEXT("F63CC3194B8FA31B2F1C60AA0FD2553B")));
    if(!Player) return 6;
    Player->BecomeRelevantFunction.SetExternalMember(TEXT("SetupStanceTransitionAnims"),UCharacterAnimLayerInstance::StaticClass());
    Player->UpdateFunction.SetExternalMember(TEXT("UpdateStanceTransitionAnims"),UCharacterAnimLayerInstance::StaticClass());
    Player->Node.SetLoopAnimation(false);
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Layer);
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Main);
    const TCHAR* Names[]={TEXT("ABP_Helldiver_Layer"),TEXT("ABP_Helldiver_Rifle"),TEXT("ABP_Helldiver_Pistol"),TEXT("ABP_Helldiver_Unarmed"),TEXT("ABP_Helldiver_MK5")};
    for(const auto* Name:Names)
    {
        auto* BP=LoadObject<UAnimBlueprint>(nullptr,*(Prefix+Name+TEXT(".")+Name)); if(!BP) return 7;
        // Retain configured compatible clips; only repair the known foreign-skeleton defaults.
        for(const auto* Field:{TEXT("_standToProneAnim"),TEXT("_proneToStandAnim")})
        {
            auto* P=FindFProperty<FObjectPropertyBase>(BP->GeneratedClass,Field); if(!P) continue;
            auto* Clip=Cast<UAnimSequence>(P->GetObjectPropertyValue_InContainer(BP->GeneratedClass->GetDefaultObject()));
            auto* Expected=LoadObject<UAnimSequence>(nullptr,Field==FName(TEXT("_standToProneAnim")) ?
                TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/Assets/Prone/Helldiver_anim_Stand_Up_To_Prone_R.Helldiver_anim_Stand_Up_To_Prone_R"):
                TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/Assets/Prone/Helldiver_anim_Prone_Pistol_To_Stand_Up_R.Helldiver_anim_Prone_Pistol_To_Stand_Up_R"));
            if(Expected && (!Clip || Clip->GetSkeleton()!=Expected->GetSkeleton())) P->SetObjectPropertyValue_InContainer(BP->GeneratedClass->GetDefaultObject(),Expected);
        }
        FCompilerResultsLog Results;
        FKismetEditorUtilities::CompileBlueprint(BP,EBlueprintCompileOptions::SkipGarbageCollection,&Results);
        UE_LOG(LogTemp,Display,TEXT("WeaponOnly %s errors=%d warnings=%d"),Name,Results.NumErrors,Results.NumWarnings);
        if(Results.NumErrors || Results.NumWarnings) return 8;
        FString File=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("WeaponOnlyMontageRepair")/(FString(Name)+TEXT(".uasset")));
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(File),true);
        FSavePackageArgs Save; Save.TopLevelFlags=RF_Public|RF_Standalone; Save.SaveFlags=SAVE_NoError;
        if(!UPackage::SavePackage(BP->GetOutermost(),BP,*File,Save)) return 9;
    }
    return 0;
}
