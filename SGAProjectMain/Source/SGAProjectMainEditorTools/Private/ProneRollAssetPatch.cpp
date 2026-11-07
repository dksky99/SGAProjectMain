#include "ProneRollAssetPatch.h"

#include "Animation/AnimSequence.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "Animation/BlendSpace.h"
#include "Dom/JsonValue.h"
#include "Engine/Blueprint.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"

namespace CharacterAnimationAudit
{
    bool CompileForAudit(UBlueprint* Blueprint, TSharedRef<FJsonObject> Result);
    bool SaveStagedBlueprint(UBlueprint* Blueprint, TSharedRef<FJsonObject> Result,
        TFunctionRef<bool()> WriteCheckpoint);

    namespace ProneRollAssetPatch
    {
        const FName IdleField(TEXT("_proneIdleAnims"));

        struct FDirection
        {
            FName Member;
            FString PreviousPath;
            FString CandidatePath;
            UAnimSequence* Candidate = nullptr;
        };

        struct FConcrete
        {
            UBlueprint* Blueprint = nullptr;
            TSharedPtr<FJsonObject> Report;
            TMap<FString, FString> OtherDefaults;
        };

        FObjectPropertyBase* DirectionProperty(FStructProperty* IdleProperty, FName Direction)
        {
            return IdleProperty && IdleProperty->Struct
                ? FindFProperty<FObjectPropertyBase>(IdleProperty->Struct, Direction) : nullptr;
        }

        UObject* ReadDirection(UObject* CDO, FName Direction)
        {
            FStructProperty* Idle = CDO ? FindFProperty<FStructProperty>(CDO->GetClass(), IdleField) : nullptr;
            FObjectPropertyBase* Property = DirectionProperty(Idle, Direction);
            return Property ? Property->GetObjectPropertyValue_InContainer(Idle->ContainerPtrToValuePtr<void>(CDO)) : nullptr;
        }

        bool IsProneAOField(FName Name)
        {
            return Name == TEXT("_aoProneFwd") || Name == TEXT("_aoProneBwd") ||
                Name == TEXT("_aoProneLeft") || Name == TEXT("_aoProneRight");
        }

        TMap<FString, FString> SnapshotOtherDefaults(UObject* CDO, bool bExcludeProneAO = false)
        {
            TMap<FString, FString> Snapshot;
            for (TFieldIterator<FProperty> It(CDO->GetClass()); It; ++It)
            {
                const FProperty* Property = *It;
                if (Property->GetFName() == IdleField || (bExcludeProneAO && IsProneAOField(Property->GetFName())) ||
                    Property->HasAnyPropertyFlags(CPF_Transient | CPF_Deprecated)) continue;
                FString Value;
                Property->ExportTextItem_Direct(Value, Property->ContainerPtrToValuePtr<void>(CDO), nullptr, CDO, PPF_PropertyWindow);
                Snapshot.Add(Property->GetName(), MoveTemp(Value));
            }
            return Snapshot;
        }

        bool CompareOtherDefaults(const TMap<FString, FString>& Before, UObject* CDO, TSharedRef<FJsonObject> Result,
            bool bExcludeProneAO = false)
        {
            const TMap<FString, FString> After = SnapshotOtherDefaults(CDO, bExcludeProneAO);
            TArray<TSharedPtr<FJsonValue>> Changes;
            for (const auto& Pair : Before)
            {
                const FString* Value = After.Find(Pair.Key);
                if (!Value || *Value != Pair.Value) Changes.Add(MakeShared<FJsonValueString>(Pair.Key));
            }
            for (const auto& Pair : After)
                if (!Before.Contains(Pair.Key)) Changes.Add(MakeShared<FJsonValueString>(Pair.Key));
            Result->SetNumberField(TEXT("unrelated_persistent_cdo_properties_checked"), Before.Num());
            Result->SetArrayField(TEXT("unexpected_cdo_property_changes"), Changes);
            Result->SetBoolField(TEXT("unrelated_persistent_cdo_properties_unchanged"), Changes.IsEmpty());
            return Changes.IsEmpty();
        }
    }

    bool StageNaturalProneIdleDefaults(const TMap<FString, UBlueprint*>& Loaded,
        TSharedRef<FJsonObject> Result, TFunctionRef<bool()> WriteCheckpoint)
    {
        using namespace ProneRollAssetPatch;
        const auto Fail = [&Result](const FString& Message)
        {
            Result->SetStringField(TEXT("error"), Message);
            UE_LOG(LogTemp, Error, TEXT("ProneRollAssetPatch: %s"), *Message);
            return false;
        };
        Result->SetStringField(TEXT("scope"), TEXT("Candidate: change only Backward/Right/Left prone Idle defaults on Rifle/Pistol/Unarmed; existing animation assets, AO and graphs are unchanged."));
        Result->SetBoolField(TEXT("original_overwritten"), false);
        Result->SetStringField(TEXT("right_left_semantics"), TEXT("Direction is desired facing: existing ABS_Prone_Retargeted assigns +90 to Laying_Left1 and -90 to Laying_Right1."));
        const FString OldFolder = TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/Assets/Dodge/Aim/");
        const FString CandidateFolder = TEXT("/Game/Blueprints/HellDiver/Animation/Retargeted_Anim/Prone/Idle/");
        const FString ForwardPath = TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/Assets/Prone/Helldiver_Anim_Prone_Pistol_Idle_R.Helldiver_Anim_Prone_Pistol_Idle_R");
        TArray<FDirection> Directions = {
            { TEXT("Backward"), OldFolder + TEXT("Backward/Helldiveranim_Prone_Bwd_Idle.Helldiveranim_Prone_Bwd_Idle"),
                CandidateFolder + TEXT("Hdv_Male_Lying1_Reatergeted.Hdv_Male_Lying1_Reatergeted") },
            { TEXT("Right"), OldFolder + TEXT("Right/Helldiveranim_Prone_Right_Idle.Helldiveranim_Prone_Right_Idle"),
                CandidateFolder + TEXT("Hdv_Male_Laying_Left1_Reatergeted.Hdv_Male_Laying_Left1_Reatergeted") },
            { TEXT("Left"), OldFolder + TEXT("Left/Helldiveranim_Prone_Left_Idle.Helldiveranim_Prone_Left_Idle"),
                CandidateFolder + TEXT("Hdv_Male_Laying_Right1_Reatergeted.Hdv_Male_Laying_Right1_Reatergeted") }
        };
        for (FDirection& Direction : Directions)
        {
            Direction.Candidate = LoadObject<UAnimSequence>(nullptr, *Direction.CandidatePath);
            if (!Direction.Candidate || Direction.Candidate->GetPathName() != Direction.CandidatePath ||
                Direction.Candidate->AdditiveAnimType != AAT_None || Direction.Candidate->GetPlayLength() <= 0.f)
                return Fail(FString::Printf(TEXT("Expected non-additive authored Idle candidate could not be validated: %s"), *Direction.CandidatePath));
        }

        TArray<FConcrete> Concrete;
        TArray<TSharedPtr<FJsonValue>> Packages;
        Result->SetArrayField(TEXT("packages"), Packages);
        const FString BlueprintFolder = TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/");
        for (const TCHAR* Weapon : { TEXT("Rifle"), TEXT("Pistol"), TEXT("Unarmed") })
        {
            const FString PackagePath = BlueprintFolder + TEXT("ABP_Helldiver_") + Weapon;
            UBlueprint* const* Found = Loaded.Find(PackagePath);
            UBlueprint* Blueprint = Found ? *Found : nullptr;
            if (!Blueprint || Blueprint->GetOutermost()->GetName() != PackagePath || !Blueprint->GeneratedClass)
                return Fail(FString::Printf(TEXT("The exact existing concrete Blueprint must be loaded: %s"), *PackagePath));
            UObject* CDO = Blueprint->GeneratedClass->GetDefaultObject();
            FStructProperty* Idle = FindFProperty<FStructProperty>(CDO->GetClass(), IdleField);
            if (!Idle || !Idle->Struct || Idle->Struct->GetFName() != TEXT("DirectionalAnimations"))
                return Fail(TEXT("The expected native FDirectionalAnimations Idle property was not found."));
            int32 DirectionMemberCount = 0;
            for (TFieldIterator<FProperty> It(Idle->Struct); It; ++It)
            {
                const FName Name = It->GetFName();
                if (!CastField<FObjectPropertyBase>(*It) ||
                    (Name != TEXT("Forward") && Name != TEXT("Backward") && Name != TEXT("Right") && Name != TEXT("Left")))
                    return Fail(TEXT("The native directional Idle struct differs from its four-object-field baseline."));
                ++DirectionMemberCount;
            }
            if (DirectionMemberCount != 4)
                return Fail(TEXT("The native directional Idle struct does not contain exactly four direction members."));
            UAnimSequence* Forward = Cast<UAnimSequence>(ReadDirection(CDO, TEXT("Forward")));
            if (!Forward || Forward->GetPathName() != ForwardPath || !Forward->GetSkeleton())
                return Fail(FString::Printf(TEXT("Forward Idle differs from the audited baseline on %s; no package staged."), Weapon));
            for (const FDirection& Direction : Directions)
            {
                UObject* Current = ReadDirection(CDO, Direction.Member);
                if (!Current || Current->GetPathName() != Direction.PreviousPath)
                    return Fail(FString::Printf(TEXT("%s Idle differs from the audited baseline on %s; no package staged."), *Direction.Member.ToString(), Weapon));
                if (Direction.Candidate->GetSkeleton() != Forward->GetSkeleton())
                    return Fail(FString::Printf(TEXT("Candidate Idle skeleton differs from the active Forward Idle: %s"), *Direction.CandidatePath));
            }
            FConcrete Data;
            Data.Blueprint = Blueprint;
            Data.Report = MakeShared<FJsonObject>();
            Data.Report->SetStringField(TEXT("blueprint"), Blueprint->GetPathName());
            Data.Report->SetStringField(TEXT("forward_unchanged"), ForwardPath);
            Data.OtherDefaults = SnapshotOtherDefaults(CDO);
            Concrete.Add(MoveTemp(Data));
            Packages.Add(MakeShared<FJsonValueObject>(Concrete.Last().Report.ToSharedRef()));
        }
        Result->SetArrayField(TEXT("packages"), Packages);
        Result->SetStringField(TEXT("phase"), TEXT("all_candidates_and_baselines_validated_before_mutation"));
        if (!WriteCheckpoint()) return Fail(TEXT("Could not checkpoint the validated candidate; no defaults changed."));

        for (FConcrete& Data : Concrete)
        {
            Data.Blueprint->Modify();
            UObject* CDO = Data.Blueprint->GeneratedClass->GetDefaultObject();
            CDO->Modify();
            FStructProperty* Idle = FindFProperty<FStructProperty>(CDO->GetClass(), IdleField);
            void* IdleData = Idle->ContainerPtrToValuePtr<void>(CDO);
            TArray<TSharedPtr<FJsonValue>> Replacements;
            for (const FDirection& Direction : Directions)
            {
                DirectionProperty(Idle, Direction.Member)->SetObjectPropertyValue_InContainer(IdleData, Direction.Candidate);
                TSharedRef<FJsonObject> Row = MakeShared<FJsonObject>();
                Row->SetStringField(TEXT("direction"), Direction.Member.ToString());
                Row->SetStringField(TEXT("before"), Direction.PreviousPath);
                Row->SetStringField(TEXT("candidate"), Direction.CandidatePath);
                Replacements.Add(MakeShared<FJsonValueObject>(Row));
            }
            Data.Report->SetArrayField(TEXT("idle_replacements"), Replacements);
        }
        for (FConcrete& Data : Concrete)
        {
            if (!CompileForAudit(Data.Blueprint, Data.Report.ToSharedRef()))
                return Fail(TEXT("A concrete layer candidate did not compile; no candidate package staged."));
            UObject* CDO = Data.Blueprint->GeneratedClass->GetDefaultObject();
            if (GetPathNameSafe(ReadDirection(CDO, TEXT("Forward"))) != ForwardPath)
                return Fail(TEXT("Forward Idle changed during candidate compilation; no candidate package staged."));
            for (const FDirection& Direction : Directions)
                if (ReadDirection(CDO, Direction.Member) != Direction.Candidate)
                    return Fail(TEXT("Candidate Idle defaults did not survive compilation; no candidate package staged."));
            if (!CompareOtherDefaults(Data.OtherDefaults, CDO, Data.Report.ToSharedRef()))
                return Fail(TEXT("Unrelated persistent CDO defaults changed during candidate compilation; no candidate package staged."));
        }
        Result->SetStringField(TEXT("phase"), TEXT("all_candidates_compiled_and_defaults_verified_before_staging"));
        if (!WriteCheckpoint()) return Fail(TEXT("Could not checkpoint candidate compilation; no candidate package staged."));
        for (FConcrete& Data : Concrete)
            if (!SaveStagedBlueprint(Data.Blueprint, Data.Report.ToSharedRef(), WriteCheckpoint))
                return Fail(TEXT("Could not save a candidate layer to staging; Content assets were not overwritten."));
        Result->SetBoolField(TEXT("all_candidates_staged"), true);
        return true;
    }

    bool StageCanonicalProneDefaults(const TMap<FString, UBlueprint*>& Loaded,
        const TMap<FString, UAnimSequence*>& Idle, const TMap<FString, UBlendSpace*>& AO,
        TSharedRef<FJsonObject> Result, TFunctionRef<bool()> WriteCheckpoint)
    {
        using namespace ProneRollAssetPatch;
        const auto Fail = [&Result](const FString& Message)
        {
            Result->SetStringField(TEXT("error"), Message);
            UE_LOG(LogTemp, Error, TEXT("ProneRollAssetPatch Canonical: %s"), *Message);
            return false;
        };
        struct FMapping
        {
            FString Direction;
            FName AOField;
            FString PreviousIdlePath;
            FString PreviousAOPath;
            UAnimSequence* Idle = nullptr;
            UBlendSpace* AO = nullptr;
        };
        const FString Folder = TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/");
        const FString AimFolder = Folder + TEXT("Assets/Dodge/Aim/");
        const FString AOFolder = Folder + TEXT("AimOffset/");
        TArray<FMapping> Mappings = {
            { TEXT("Forward"), TEXT("_aoProneFwd"),
                Folder + TEXT("Assets/Prone/Helldiver_Anim_Prone_Pistol_Idle_R.Helldiver_Anim_Prone_Pistol_Idle_R"),
                AOFolder + TEXT("AO_ProneFwd.AO_ProneFwd") },
            { TEXT("Right"), TEXT("_aoProneRight"),
                AimFolder + TEXT("Right/Helldiveranim_Prone_Right_Idle.Helldiveranim_Prone_Right_Idle"),
                AOFolder + TEXT("AO_ProneRight.AO_ProneRight") },
            { TEXT("Backward"), TEXT("_aoProneBwd"),
                AimFolder + TEXT("Backward/Helldiveranim_Prone_Bwd_Idle.Helldiveranim_Prone_Bwd_Idle"),
                AOFolder + TEXT("AO_ProneBwd.AO_ProneBwd") },
            { TEXT("Left"), TEXT("_aoProneLeft"),
                AimFolder + TEXT("Left/Helldiveranim_Prone_Left_Idle.Helldiveranim_Prone_Left_Idle"),
                AOFolder + TEXT("AO_ProneLeft.AO_ProneLeft") }
        };
        Result->SetStringField(TEXT("scope"), TEXT("Stage only four prone Idle and four prone AO defaults on Rifle/Pistol/Unarmed; no Main Blueprint, native logic, graph, or original animation asset changes."));
        Result->SetBoolField(TEXT("original_overwritten"), false);
        Result->SetBoolField(TEXT("natural_idle_only_candidate_applied"), false);
        if (Idle.Num() != 4 || AO.Num() != 4)
            return Fail(TEXT("Canonical Idle and AO maps must each contain exactly the four direction keys."));
        for (FMapping& Mapping : Mappings)
        {
            UAnimSequence* const* IdleCandidate = Idle.Find(Mapping.Direction);
            UBlendSpace* const* AOCandidate = AO.Find(Mapping.Direction);
            Mapping.Idle = IdleCandidate ? *IdleCandidate : nullptr;
            Mapping.AO = AOCandidate ? *AOCandidate : nullptr;
            if (!Mapping.Idle || !Mapping.AO || !Mapping.Idle->GetSkeleton() ||
                Mapping.Idle->AdditiveAnimType != AAT_None || Mapping.Idle->GetPlayLength() <= 0.f ||
                !Cast<UAimOffsetBlendSpace>(Mapping.AO) || Mapping.AO->GetSkeleton() != Mapping.Idle->GetSkeleton() ||
                Mapping.AO->GetBlendSamples().IsEmpty())
                return Fail(FString::Printf(TEXT("Canonical %s Idle/AO candidate is missing or has an invalid animation type or skeleton."), *Mapping.Direction));
            for (const FBlendSample& Sample : Mapping.AO->GetBlendSamples())
                if (!Sample.Animation || Sample.Animation->GetSkeleton() != Mapping.Idle->GetSkeleton() ||
                    Sample.Animation->AdditiveAnimType != AAT_RotationOffsetMeshSpace)
                    return Fail(FString::Printf(TEXT("Canonical %s AO sample is missing or has an invalid skeleton/additive type."), *Mapping.Direction));
        }

        TArray<FConcrete> Concrete;
        TArray<TSharedPtr<FJsonValue>> Packages;
        Result->SetArrayField(TEXT("packages"), Packages);
        for (const TCHAR* Weapon : { TEXT("Rifle"), TEXT("Pistol"), TEXT("Unarmed") })
        {
            const FString PackagePath = Folder + TEXT("ABP_Helldiver_") + Weapon;
            UBlueprint* const* Found = Loaded.Find(PackagePath);
            UBlueprint* Blueprint = Found ? *Found : nullptr;
            if (!Blueprint || Blueprint->GetOutermost()->GetName() != PackagePath || !Blueprint->GeneratedClass)
                return Fail(FString::Printf(TEXT("The exact existing concrete Blueprint must be loaded: %s"), *PackagePath));
            UObject* CDO = Blueprint->GeneratedClass->GetDefaultObject();
            FStructProperty* IdleProperty = FindFProperty<FStructProperty>(CDO->GetClass(), IdleField);
            if (!IdleProperty || !IdleProperty->Struct || IdleProperty->Struct->GetFName() != TEXT("DirectionalAnimations"))
                return Fail(TEXT("Expected native FDirectionalAnimations prone Idle field was not found."));
            int32 MemberCount = 0;
            for (TFieldIterator<FProperty> It(IdleProperty->Struct); It; ++It)
            {
                const FName Name = It->GetFName();
                if (!CastField<FObjectPropertyBase>(*It) ||
                    (Name != TEXT("Forward") && Name != TEXT("Right") && Name != TEXT("Backward") && Name != TEXT("Left")))
                    return Fail(TEXT("Native prone Idle struct differs from its four-object-field baseline."));
                ++MemberCount;
            }
            if (MemberCount != 4) return Fail(TEXT("Native prone Idle struct does not have exactly four members."));
            for (const FMapping& Mapping : Mappings)
            {
                UAnimSequence* PreviousIdle = Cast<UAnimSequence>(ReadDirection(CDO, FName(*Mapping.Direction)));
                FObjectPropertyBase* AOProperty = FindFProperty<FObjectPropertyBase>(CDO->GetClass(), Mapping.AOField);
                UObject* PreviousAO = AOProperty ? AOProperty->GetObjectPropertyValue_InContainer(CDO) : nullptr;
                if (!PreviousIdle || PreviousIdle->GetPathName() != Mapping.PreviousIdlePath ||
                    !PreviousAO || PreviousAO->GetPathName() != Mapping.PreviousAOPath)
                    return Fail(FString::Printf(TEXT("%s prone Idle/AO defaults differ from the exact audited baseline on %s; no candidate staged."), *Mapping.Direction, Weapon));
                if (PreviousIdle->GetSkeleton() != Mapping.Idle->GetSkeleton() ||
                    !Mapping.Idle->IsA(DirectionProperty(IdleProperty, FName(*Mapping.Direction))->PropertyClass) ||
                    !Mapping.AO->IsA(AOProperty->PropertyClass))
                    return Fail(FString::Printf(TEXT("Canonical %s candidates do not match the existing skeleton or property classes on %s."), *Mapping.Direction, Weapon));
            }
            FConcrete Data;
            Data.Blueprint = Blueprint;
            Data.Report = MakeShared<FJsonObject>();
            Data.Report->SetStringField(TEXT("blueprint"), Blueprint->GetPathName());
            Data.OtherDefaults = SnapshotOtherDefaults(CDO, true);
            Concrete.Add(MoveTemp(Data));
            Packages.Add(MakeShared<FJsonValueObject>(Concrete.Last().Report.ToSharedRef()));
        }
        Result->SetArrayField(TEXT("packages"), Packages);
        Result->SetStringField(TEXT("phase"), TEXT("all_canonical_candidates_and_baselines_validated_before_mutation"));
        if (!WriteCheckpoint()) return Fail(TEXT("Could not checkpoint canonical candidates; no defaults changed."));

        for (FConcrete& Data : Concrete)
        {
            Data.Blueprint->Modify();
            UObject* CDO = Data.Blueprint->GeneratedClass->GetDefaultObject();
            CDO->Modify();
            FStructProperty* IdleProperty = FindFProperty<FStructProperty>(CDO->GetClass(), IdleField);
            void* IdleData = IdleProperty->ContainerPtrToValuePtr<void>(CDO);
            TArray<TSharedPtr<FJsonValue>> Replacements;
            for (const FMapping& Mapping : Mappings)
            {
                DirectionProperty(IdleProperty, FName(*Mapping.Direction))->SetObjectPropertyValue_InContainer(IdleData, Mapping.Idle);
                FindFProperty<FObjectPropertyBase>(CDO->GetClass(), Mapping.AOField)->SetObjectPropertyValue_InContainer(CDO, Mapping.AO);
                TSharedRef<FJsonObject> Row = MakeShared<FJsonObject>();
                Row->SetStringField(TEXT("direction"), Mapping.Direction);
                Row->SetStringField(TEXT("idle_before"), Mapping.PreviousIdlePath);
                Row->SetStringField(TEXT("idle_candidate"), Mapping.Idle->GetPathName());
                Row->SetStringField(TEXT("ao_before"), Mapping.PreviousAOPath);
                Row->SetStringField(TEXT("ao_candidate"), Mapping.AO->GetPathName());
                Replacements.Add(MakeShared<FJsonValueObject>(Row));
            }
            Data.Report->SetArrayField(TEXT("prone_replacements"), Replacements);
        }
        for (FConcrete& Data : Concrete)
        {
            if (!CompileForAudit(Data.Blueprint, Data.Report.ToSharedRef()))
                return Fail(TEXT("A canonical layer did not compile; no candidate layer staged."));
            UObject* CDO = Data.Blueprint->GeneratedClass->GetDefaultObject();
            for (const FMapping& Mapping : Mappings)
            {
                FObjectPropertyBase* AOProperty = FindFProperty<FObjectPropertyBase>(CDO->GetClass(), Mapping.AOField);
                if (ReadDirection(CDO, FName(*Mapping.Direction)) != Mapping.Idle || !AOProperty ||
                    AOProperty->GetObjectPropertyValue_InContainer(CDO) != Mapping.AO)
                    return Fail(TEXT("Canonical prone Idle/AO defaults did not survive compilation; no candidate layer staged."));
            }
            if (!CompareOtherDefaults(Data.OtherDefaults, CDO, Data.Report.ToSharedRef(), true))
                return Fail(TEXT("Unrelated persistent defaults changed during canonical layer compilation; no candidate layer staged."));
        }
        Result->SetStringField(TEXT("phase"), TEXT("all_canonical_layers_compiled_and_defaults_verified_before_staging"));
        if (!WriteCheckpoint()) return Fail(TEXT("Could not checkpoint compiled canonical layers; no candidate layer staged."));
        for (FConcrete& Data : Concrete)
            if (!SaveStagedBlueprint(Data.Blueprint, Data.Report.ToSharedRef(), WriteCheckpoint))
                return Fail(TEXT("Could not save a canonical layer to staging; Content assets were not overwritten."));
        Result->SetBoolField(TEXT("all_candidates_staged"), true);
        return true;
    }

    bool StageUnifiedProneDefaults(const TMap<FString, UBlueprint*>& Loaded,
        UAnimSequence* Base, UBlendSpace* AO, TSharedRef<FJsonObject> Result,
        TFunctionRef<bool()> WriteCheckpoint)
    {
        using namespace ProneRollAssetPatch;
        const auto Fail = [&Result](const FString& Message)
        {
            Result->SetStringField(TEXT("error"), Message);
            UE_LOG(LogTemp, Error, TEXT("ProneRollAssetPatch Unified: %s"), *Message);
            return false;
        };
        struct FMapping
        {
            FName Direction;
            FName AOField;
            FString PreviousIdlePath;
            FString PreviousAOPath;
        };
        const FString Folder = TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/");
        const FString CanonicalFolder = Folder + TEXT("ProneCanonical/");
        const TArray<FMapping> Mappings = {
            { TEXT("Forward"), TEXT("_aoProneFwd"),
                CanonicalFolder + TEXT("Forward/Helldiver_Anim_Prone_Pistol_Idle_R.Helldiver_Anim_Prone_Pistol_Idle_R"),
                CanonicalFolder + TEXT("Forward/AO_ProneCanonical_Forward.AO_ProneCanonical_Forward") },
            { TEXT("Right"), TEXT("_aoProneRight"),
                CanonicalFolder + TEXT("Right/Helldiveranim_Prone_Left_Idle.Helldiveranim_Prone_Left_Idle"),
                CanonicalFolder + TEXT("Right/AO_ProneCanonical_Right.AO_ProneCanonical_Right") },
            { TEXT("Backward"), TEXT("_aoProneBwd"),
                CanonicalFolder + TEXT("Backward/Helldiveranim_Prone_Bwd_Idle.Helldiveranim_Prone_Bwd_Idle"),
                CanonicalFolder + TEXT("Backward/AO_ProneCanonical_Backward.AO_ProneCanonical_Backward") },
            { TEXT("Left"), TEXT("_aoProneLeft"),
                CanonicalFolder + TEXT("Left/Helldiveranim_Prone_Right_Idle.Helldiveranim_Prone_Right_Idle"),
                CanonicalFolder + TEXT("Left/AO_ProneCanonical_Left.AO_ProneCanonical_Left") }
        };
        Result->SetStringField(TEXT("scope"), TEXT("Migrate only the exact applied canonical defaults to one shared prone Base and AO on Rifle/Pistol/Unarmed; no Main, native selection, graph, or source animation changes."));
        Result->SetStringField(TEXT("reason"), TEXT("A shared Idle and additive reference makes sector selection order irrelevant; authored AO samples supply continuous full-body roll and aim."));
        Result->SetBoolField(TEXT("original_overwritten"), false);
        if (!Base || !AO || !Base->GetSkeleton() || Base->AdditiveAnimType != AAT_None ||
            Base->GetPlayLength() <= 0.f || !Cast<UAimOffsetBlendSpace>(AO) ||
            AO->GetSkeleton() != Base->GetSkeleton() || AO->GetBlendSamples().IsEmpty())
            return Fail(TEXT("The unified Base/AO must be non-null, non-additive Base plus AimOffset with the same skeleton and valid samples."));
        for (const FBlendSample& Sample : AO->GetBlendSamples())
            if (!Sample.Animation || Sample.Animation->GetSkeleton() != Base->GetSkeleton() ||
                Sample.Animation->AdditiveAnimType != AAT_RotationOffsetMeshSpace || Sample.Animation->RefPoseSeq.Get() != Base)
                return Fail(TEXT("Every unified AO sample must be mesh-space additive with the supplied shared Base as its exact reference."));
        Result->SetStringField(TEXT("shared_base"), Base->GetPathName());
        Result->SetStringField(TEXT("shared_ao"), AO->GetPathName());
        Result->SetNumberField(TEXT("shared_ao_sample_slots"), AO->GetBlendSamples().Num());

        TArray<FConcrete> Concrete;
        TArray<TSharedPtr<FJsonValue>> Packages;
        Result->SetArrayField(TEXT("packages"), Packages);
        for (const TCHAR* Weapon : { TEXT("Rifle"), TEXT("Pistol"), TEXT("Unarmed") })
        {
            const FString PackagePath = Folder + TEXT("ABP_Helldiver_") + Weapon;
            UBlueprint* const* Found = Loaded.Find(PackagePath);
            UBlueprint* Blueprint = Found ? *Found : nullptr;
            if (!Blueprint || Blueprint->GetOutermost()->GetName() != PackagePath || !Blueprint->GeneratedClass)
                return Fail(FString::Printf(TEXT("The exact concrete Blueprint must be loaded: %s"), *PackagePath));
            UObject* CDO = Blueprint->GeneratedClass->GetDefaultObject();
            FStructProperty* IdleProperty = FindFProperty<FStructProperty>(CDO->GetClass(), IdleField);
            if (!IdleProperty || !IdleProperty->Struct || IdleProperty->Struct->GetFName() != TEXT("DirectionalAnimations"))
                return Fail(TEXT("Expected native FDirectionalAnimations prone Idle field was not found."));
            int32 MemberCount = 0;
            for (TFieldIterator<FProperty> It(IdleProperty->Struct); It; ++It)
            {
                const FName Name = It->GetFName();
                if (!CastField<FObjectPropertyBase>(*It) ||
                    (Name != TEXT("Forward") && Name != TEXT("Right") && Name != TEXT("Backward") && Name != TEXT("Left")))
                    return Fail(TEXT("Native prone Idle struct differs from its four-object-field baseline."));
                ++MemberCount;
            }
            if (MemberCount != 4) return Fail(TEXT("Native prone Idle struct does not contain exactly four direction members."));
            for (const FMapping& Mapping : Mappings)
            {
                UAnimSequence* PreviousIdle = Cast<UAnimSequence>(ReadDirection(CDO, Mapping.Direction));
                FObjectPropertyBase* AOProperty = FindFProperty<FObjectPropertyBase>(CDO->GetClass(), Mapping.AOField);
                UObject* PreviousAO = AOProperty ? AOProperty->GetObjectPropertyValue_InContainer(CDO) : nullptr;
                if (!PreviousIdle || PreviousIdle->GetPathName() != Mapping.PreviousIdlePath ||
                    !PreviousAO || PreviousAO->GetPathName() != Mapping.PreviousAOPath)
                    return Fail(FString::Printf(TEXT("%s Idle/AO defaults are not the exact applied canonical paths on %s; no unified layer staged."), *Mapping.Direction.ToString(), Weapon));
                if (PreviousIdle->GetSkeleton() != Base->GetSkeleton() ||
                    !Base->IsA(DirectionProperty(IdleProperty, Mapping.Direction)->PropertyClass) || !AO->IsA(AOProperty->PropertyClass))
                    return Fail(FString::Printf(TEXT("Unified Base/AO does not match the existing skeleton or property classes on %s."), Weapon));
            }
            FConcrete Data;
            Data.Blueprint = Blueprint;
            Data.Report = MakeShared<FJsonObject>();
            Data.Report->SetStringField(TEXT("blueprint"), Blueprint->GetPathName());
            Data.OtherDefaults = SnapshotOtherDefaults(CDO, true);
            Concrete.Add(MoveTemp(Data));
            Packages.Add(MakeShared<FJsonValueObject>(Concrete.Last().Report.ToSharedRef()));
        }
        Result->SetArrayField(TEXT("packages"), Packages);
        Result->SetStringField(TEXT("phase"), TEXT("all_unified_candidates_and_canonical_baselines_validated_before_mutation"));
        if (!WriteCheckpoint()) return Fail(TEXT("Could not checkpoint the unified candidate; no defaults changed."));

        for (FConcrete& Data : Concrete)
        {
            Data.Blueprint->Modify();
            UObject* CDO = Data.Blueprint->GeneratedClass->GetDefaultObject();
            CDO->Modify();
            FStructProperty* IdleProperty = FindFProperty<FStructProperty>(CDO->GetClass(), IdleField);
            void* IdleData = IdleProperty->ContainerPtrToValuePtr<void>(CDO);
            TArray<TSharedPtr<FJsonValue>> Replacements;
            for (const FMapping& Mapping : Mappings)
            {
                DirectionProperty(IdleProperty, Mapping.Direction)->SetObjectPropertyValue_InContainer(IdleData, Base);
                FindFProperty<FObjectPropertyBase>(CDO->GetClass(), Mapping.AOField)->SetObjectPropertyValue_InContainer(CDO, AO);
                TSharedRef<FJsonObject> Row = MakeShared<FJsonObject>();
                Row->SetStringField(TEXT("direction"), Mapping.Direction.ToString());
                Row->SetStringField(TEXT("idle_before"), Mapping.PreviousIdlePath);
                Row->SetStringField(TEXT("idle_shared_base"), Base->GetPathName());
                Row->SetStringField(TEXT("ao_before"), Mapping.PreviousAOPath);
                Row->SetStringField(TEXT("ao_shared"), AO->GetPathName());
                Replacements.Add(MakeShared<FJsonValueObject>(Row));
            }
            Data.Report->SetArrayField(TEXT("unified_prone_replacements"), Replacements);
        }
        for (FConcrete& Data : Concrete)
        {
            if (!CompileForAudit(Data.Blueprint, Data.Report.ToSharedRef()))
                return Fail(TEXT("A unified layer did not compile; no unified layer staged."));
            UObject* CDO = Data.Blueprint->GeneratedClass->GetDefaultObject();
            for (const FMapping& Mapping : Mappings)
            {
                FObjectPropertyBase* AOProperty = FindFProperty<FObjectPropertyBase>(CDO->GetClass(), Mapping.AOField);
                if (ReadDirection(CDO, Mapping.Direction) != Base || !AOProperty || AOProperty->GetObjectPropertyValue_InContainer(CDO) != AO)
                    return Fail(TEXT("Shared prone Idle/AO defaults did not survive compilation; no unified layer staged."));
            }
            if (!CompareOtherDefaults(Data.OtherDefaults, CDO, Data.Report.ToSharedRef(), true))
                return Fail(TEXT("Unrelated persistent defaults changed during unified layer compilation; no unified layer staged."));
        }
        Result->SetStringField(TEXT("phase"), TEXT("all_unified_layers_compiled_and_defaults_verified_before_staging"));
        if (!WriteCheckpoint()) return Fail(TEXT("Could not checkpoint the compiled unified layers; no unified layer staged."));
        for (FConcrete& Data : Concrete)
            if (!SaveStagedBlueprint(Data.Blueprint, Data.Report.ToSharedRef(), WriteCheckpoint))
                return Fail(TEXT("Could not save a unified layer to staging; Content assets were not overwritten."));
        Result->SetBoolField(TEXT("all_candidates_staged"), true);
        Result->SetBoolField(TEXT("sector_selection_uses_identical_base_and_ao"), true);
        return true;
    }
}
