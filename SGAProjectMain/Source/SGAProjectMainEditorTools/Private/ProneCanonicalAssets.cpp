#include "ProneCanonicalAssets.h"

#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Animation/Skeleton.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "HAL/FileManager.h"
#include "Misc/FeedbackContext.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "ReferenceSkeleton.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

namespace ProneCanonicalAssets
{
    using FObject = TSharedRef<FJsonObject>;
    using FValues = TArray<TSharedPtr<FJsonValue>>;
    const FString DestinationRoot = TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/ProneCanonical/");
    const FString SourceRoot = TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/");

    FObject Vector(const FVector& V)
    {
        FObject Out = MakeShared<FJsonObject>();
        Out->SetNumberField(TEXT("x"), V.X);
        Out->SetNumberField(TEXT("y"), V.Y);
        Out->SetNumberField(TEXT("z"), V.Z);
        return Out;
    }

    double CanonicalYaw(double Yaw)
    {
        Yaw = FMath::UnwindDegrees(Yaw);
        // +180 and -180 describe one quaternion/clip. Their distinct AO sample
        // coordinates remain separate so the original seam topology survives.
        if (FMath::IsNearlyEqual(FMath::Abs(Yaw), 180.0, 0.001)) return 180.0;
        return FMath::IsNearlyZero(Yaw, 0.001) ? 0.0 : Yaw;
    }

    FString PackageFilename(const FString& PackageName)
    {
        return FPaths::ConvertRelativePathToFull(FPackageName::LongPackageNameToFilename(
            PackageName, FPackageName::GetAssetPackageExtension()));
    }

    FString StagedFilename(const FString& PackageName)
    {
        check(PackageName.StartsWith(TEXT("/Game/")));
        return FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectSavedDir(),
            TEXT("CharacterAnimationAudit/CanonicalStaged"), PackageName.Mid(6) + TEXT(".uasset")));
    }

    bool RawPose(UAnimSequence* Animation, TArray<FTransform>& OutPose, FString& Error)
    {
        const IAnimationDataModel* Model = Animation ? Animation->GetDataModel() : nullptr;
        const USkeleton* Skeleton = Animation ? Animation->GetSkeleton() : nullptr;
        if (!Model || !Skeleton || Model->GetNumberOfKeys() < 1)
        {
            Error = FString::Printf(TEXT("Missing raw animation model/skeleton/keys: %s"), *GetPathNameSafe(Animation));
            return false;
        }
        const FReferenceSkeleton& Ref = Skeleton->GetReferenceSkeleton();
        OutPose.SetNum(Ref.GetNum());
        for (int32 Bone = 0; Bone < Ref.GetNum(); ++Bone)
        {
            const FName Name = Ref.GetBoneName(Bone);
            const FTransform Local = Model->IsValidBoneTrackName(Name)
                ? Model->GetBoneTrackTransform(Name, FFrameNumber(0)) : Ref.GetRefBonePose()[Bone];
            const int32 Parent = Ref.GetParentIndex(Bone);
            OutPose[Bone] = Parent == INDEX_NONE ? Local : Local * OutPose[Parent];
            OutPose[Bone].NormalizeRotation();
            if (OutPose[Bone].ContainsNaN())
            {
                Error = FString::Printf(TEXT("Non-finite raw pose: %s, %s"), *Animation->GetPathName(), *Name.ToString());
                return false;
            }
        }
        return true;
    }

    bool Pelvis(UAnimSequence* Animation, FVector& OutPosition, FString& Error)
    {
        TArray<FTransform> Pose;
        if (!RawPose(Animation, Pose, Error)) return false;
        const int32 Bone = Animation->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(TEXT("pelvis"));
        if (Bone == INDEX_NONE) { Error = TEXT("The source skeleton has no pelvis bone."); return false; }
        OutPosition = Pose[Bone].GetTranslation();
        return true;
    }

    struct FSequenceRecipe
    {
        UAnimSequence* Source = nullptr;
        UAnimSequence* Copy = nullptr;
        FString Direction;
        FString PackageName;
        double Yaw = 0.0;
        FObject Manifest = MakeShared<FJsonObject>();
    };

    struct FDirectionPlan
    {
        FString TargetDirection;
        FString SourceDirection;
        double CenterYaw = 0.0;
        UBlendSpace* SourceAO = nullptr;
        UBlendSpace* CopyAO = nullptr;
        int32 ReferenceRecipe = INDEX_NONE;
        int32 IdleRecipe = INDEX_NONE;
        TArray<FBlendSample> Samples;
        TArray<int32> SampleRecipes;
        FString PackageName;
        FObject Manifest = MakeShared<FJsonObject>();
    };

    bool DestinationAvailable(const FString& PackageName, FString& Error)
    {
        if (FindPackage(nullptr, *PackageName) || FPackageName::DoesPackageExist(PackageName)
            || IFileManager::Get().FileExists(*PackageFilename(PackageName))
            || IFileManager::Get().FileExists(*StagedFilename(PackageName)))
        {
            Error = TEXT("Refusing to overwrite an existing canonical or staged destination: ") + PackageName;
            return false;
        }
        return true;
    }

    bool TransformSequence(FSequenceRecipe& Recipe, UAnimSequence* CanonicalReference,
        const FVector& CommonCenter, FString& Error)
    {
        UAnimSequence* Source = Recipe.Source;
        FVector SourcePelvis;
        if (!Pelvis(Source, SourcePelvis, Error)) return false;
        const FVector Pivot(SourcePelvis.X, SourcePelvis.Y, 0.0);
        const FVector Target(CommonCenter.X, CommonCenter.Y, 0.0);
        const FQuat Correction = FRotator(0.0, Recipe.Yaw, 0.0).Quaternion();
        Source->WaitOnExistingCompression();
        UPackage* Package = CreatePackage(*Recipe.PackageName);
        if (!Package) { Error = TEXT("CreatePackage failed: ") + Recipe.PackageName; return false; }
        const FString AssetName = FPackageName::GetLongPackageAssetName(Recipe.PackageName);
        Recipe.Copy = DuplicateObject<UAnimSequence>(Source, Package, *AssetName);
        UAnimSequence* Copy = Recipe.Copy;
        if (!Copy || Copy->GetDataModel() == Source->GetDataModel())
        {
            Error = TEXT("Animation duplication failed or reused its source data model: ") + Recipe.PackageName;
            return false;
        }
        Copy->SetFlags(RF_Public | RF_Standalone);
        Copy->Modify();
        if (CanonicalReference && Copy->AdditiveAnimType != AAT_None) Copy->RefPoseSeq = CanonicalReference;
        const FReferenceSkeleton& Ref = Copy->GetSkeleton()->GetReferenceSkeleton();
        const FName RootName = Ref.GetBoneName(0);
        const IAnimationDataModel* Model = Copy->GetDataModel();
        if (!Model->IsValidBoneTrackName(RootName))
        {
            Error = TEXT("A source animation has no editable root track: ") + Source->GetPathName();
            return false;
        }
        TArray<FTransform> RootKeys;
        Model->GetBoneTrackTransforms(RootName, RootKeys);
        if (RootKeys.Num() != Model->GetNumberOfKeys())
        {
            Error = TEXT("Root key count does not match the animation model: ") + Source->GetPathName();
            return false;
        }
        TArray<FVector> Positions, Scales;
        TArray<FQuat> Rotations;
        Positions.Reserve(RootKeys.Num()); Scales.Reserve(RootKeys.Num()); Rotations.Reserve(RootKeys.Num());
        for (const FTransform& Key : RootKeys)
        {
            const FVector Position = Correction.RotateVector(Key.GetTranslation() - Pivot) + Target;
            FQuat Rotation = Correction * Key.GetRotation();
            Rotation.Normalize();
            if (Position.ContainsNaN() || Rotation.ContainsNaN())
            {
                Error = TEXT("Non-finite canonical root key: ") + Source->GetPathName();
                return false;
            }
            Positions.Add(Position); Rotations.Add(Rotation); Scales.Add(Key.GetScale3D());
        }
        IAnimationDataController& Controller = Copy->GetController();
        Controller.OpenBracket(FText::FromString(TEXT("Bake constant prone authoring-space transform")), false);
        const bool bChanged = Controller.SetBoneTrackKeys(RootName, Positions, Rotations, Scales, false);
        Controller.CloseBracket(false);
        if (!bChanged) { Error = TEXT("SetBoneTrackKeys failed: ") + Recipe.PackageName; return false; }
        Copy->CacheDerivedDataForCurrentPlatform();
        Copy->WaitOnExistingCompression();
        Copy->MarkPackageDirty();

        FVector NewPelvis;
        if (!Pelvis(Copy, NewPelvis, Error)) return false;
        if (!FVector2D(NewPelvis.X, NewPelvis.Y).Equals(FVector2D(CommonCenter.X, CommonCenter.Y), 0.01)
            || !FMath::IsNearlyEqual(NewPelvis.Z, SourcePelvis.Z, 0.01))
        {
            Error = TEXT("The representative pelvis-center / unchanged-Z contract failed: ") + Recipe.PackageName;
            return false;
        }
        // Verify that this authoring repair did not alter any non-root local
        // tracks, including breathing, recoil, arm motion and the virtual bone.
        const IAnimationDataModel* SourceModel = Source->GetDataModel();
        for (int32 Bone = 1; Bone < Ref.GetNum(); ++Bone)
        {
            const FName Name = Ref.GetBoneName(Bone);
            if (SourceModel->IsValidBoneTrackName(Name) != Model->IsValidBoneTrackName(Name))
            {
                Error = TEXT("A non-root track binding changed: ") + Name.ToString();
                return false;
            }
            if (!SourceModel->IsValidBoneTrackName(Name)) continue;
            TArray<FTransform> Before, After;
            SourceModel->GetBoneTrackTransforms(Name, Before);
            Model->GetBoneTrackTransforms(Name, After);
            if (Before.Num() != After.Num()) { Error = TEXT("A non-root key count changed."); return false; }
            for (int32 Key = 0; Key < Before.Num(); ++Key)
                if (!Before[Key].Equals(After[Key], 0.0001))
                {
                    Error = TEXT("A non-root local key changed: ") + Name.ToString();
                    return false;
                }
        }
        Recipe.Manifest->SetStringField(TEXT("asset"), Copy->GetPathName());
        Recipe.Manifest->SetObjectField(TEXT("source_pelvis_frame0_component"), Vector(SourcePelvis));
        Recipe.Manifest->SetObjectField(TEXT("canonical_pelvis_frame0_component"), Vector(NewPelvis));
        Recipe.Manifest->SetObjectField(TEXT("constant_pivot_component_xy"), Vector(Pivot));
        Recipe.Manifest->SetObjectField(TEXT("constant_target_component_xy"), Vector(Target));
        Recipe.Manifest->SetNumberField(TEXT("root_key_count"), RootKeys.Num());
        Recipe.Manifest->SetBoolField(TEXT("non_root_local_keys_unchanged"), true);
        Recipe.Manifest->SetStringField(TEXT("ref_pose_sequence"), GetPathNameSafe(Copy->RefPoseSeq.Get()));
        Recipe.Manifest->SetNumberField(TEXT("additive_anim_type"), static_cast<int32>(Copy->AdditiveAnimType.GetValue()));
        Recipe.Manifest->SetBoolField(TEXT("derived_data_cached_synchronously"), true);
        return true;
    }

    bool SaveNewAsset(UObject* Asset, FObject Manifest, FString& Error)
    {
        if (!Asset || !Asset->GetOutermost()->GetName().StartsWith(DestinationRoot))
        {
            Error = TEXT("Only newly created ProneCanonical packages may be staged.");
            return false;
        }
        UPackage* Package = Asset->GetOutermost();
        const FString Filename = StagedFilename(Package->GetName());
        if (IFileManager::Get().FileExists(*Filename))
        {
            Error = TEXT("Refusing to overwrite an existing staged asset: ") + Filename;
            return false;
        }
        if (!IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true))
        {
            Error = TEXT("Could not create canonical staging directory.");
            return false;
        }
        FSavePackageArgs Args;
        Args.TopLevelFlags = RF_Public | RF_Standalone;
        Args.SaveFlags = SAVE_NoError;
        Args.Error = GWarn;
        Args.bSlowTask = false;
        const bool bSaved = UPackage::SavePackage(Package, Asset, *Filename, Args);
        Manifest->SetBoolField(TEXT("saved"), bSaved);
        Manifest->SetBoolField(TEXT("saved_to_staging"), bSaved);
        if (!bSaved) Error = TEXT("Canonical staging SavePackage failed: ") + Filename;
        return bSaved;
    }
}

bool BuildProneCanonicalAssets(const TMap<FString, UAnimSequence*>& SourceIdleByDirection,
    TMap<FString, UAnimSequence*>& NewIdleByDirection,
    TMap<FString, UBlendSpace*>& NewAOByDirection, TSharedRef<FJsonObject> Result)
{
    using namespace ProneCanonicalAssets;
    NewIdleByDirection.Reset(); NewAOByDirection.Reset();
    Result->SetBoolField(TEXT("saved"), false);
    Result->SetBoolField(TEXT("source_packages_saved_or_overwritten"), false);
    Result->SetStringField(TEXT("scope"), TEXT("New canonical animation copies only. Each clip gets one constant root rigid transform; all non-root local keys and natural frame-to-frame variation are preserved. No runtime bone constraint."));
    Result->SetStringField(TEXT("destination_root"), DestinationRoot);
    FString Error;
    const auto Fail = [&Result, &Error](const FString& Message)
    {
        Error = Message;
        Result->SetStringField(TEXT("error"), Message);
        Result->SetStringField(TEXT("phase"), TEXT("failed"));
        UE_LOG(LogTemp, Error, TEXT("ProneCanonicalAssets: %s"), *Message);
        return false;
    };
    UAnimSequence* const* ForwardIdle = SourceIdleByDirection.Find(TEXT("Forward"));
    FVector CommonCenter;
    if (!ForwardIdle || !*ForwardIdle || !Pelvis(*ForwardIdle, CommonCenter, Error))
        return Fail(Error.IsEmpty() ? TEXT("The current Forward CDO Idle is required.") : Error);
    Result->SetObjectField(TEXT("common_frame0_pelvis_component"), Vector(CommonCenter));
    Result->SetStringField(TEXT("common_center_source"), (*ForwardIdle)->GetPathName());

    TArray<FDirectionPlan> Plans;
    const auto AddPlan = [&Plans](const TCHAR* Target, const TCHAR* Source, double Yaw)
    {
        FDirectionPlan& Plan = Plans.AddDefaulted_GetRef();
        Plan.TargetDirection = Target; Plan.SourceDirection = Source; Plan.CenterYaw = Yaw;
    };
    AddPlan(TEXT("Forward"), TEXT("Forward"), 0.0);
    AddPlan(TEXT("Right"), TEXT("Left"), 90.0);
    AddPlan(TEXT("Backward"), TEXT("Backward"), 180.0);
    AddPlan(TEXT("Left"), TEXT("Right"), -90.0);
    const TMap<FString, FString> AONames = {
        { TEXT("Forward"), TEXT("AO_ProneFwd") }, { TEXT("Right"), TEXT("AO_ProneRight") },
        { TEXT("Backward"), TEXT("AO_ProneBwd") }, { TEXT("Left"), TEXT("AO_ProneLeft") }
    };
    TArray<FSequenceRecipe> Recipes;
    TMap<FString, int32> RecipeBySourceAndTransform;
    TSet<FString> PlannedPackages;
    const auto AddRecipe = [&](UAnimSequence* Source, const FString& Direction, double Yaw) -> int32
    {
        if (!Source) { Error = TEXT("A canonical source animation is null."); return INDEX_NONE; }
        Yaw = CanonicalYaw(Yaw);
        const FString Key = FString::Printf(TEXT("%s|%s|%.6f"), *Direction, *Source->GetPathName(), Yaw);
        if (const int32* Existing = RecipeBySourceAndTransform.Find(Key)) return *Existing;
        const FString PackageName = DestinationRoot + Direction + TEXT("/") + Source->GetName();
        if (PlannedPackages.Contains(PackageName))
        {
            Error = TEXT("Different source/transform recipes would collide at: ") + PackageName;
            return INDEX_NONE;
        }
        if (!DestinationAvailable(PackageName, Error)) return INDEX_NONE;
        PlannedPackages.Add(PackageName);
        FSequenceRecipe& Recipe = Recipes.AddDefaulted_GetRef();
        Recipe.Source = Source; Recipe.Direction = Direction; Recipe.Yaw = Yaw; Recipe.PackageName = PackageName;
        Recipe.Manifest->SetStringField(TEXT("source"), Source->GetPathName());
        Recipe.Manifest->SetStringField(TEXT("direction"), Direction);
        Recipe.Manifest->SetStringField(TEXT("package"), PackageName);
        Recipe.Manifest->SetStringField(TEXT("package_filename"), PackageFilename(PackageName));
        Recipe.Manifest->SetStringField(TEXT("staged_filename"), StagedFilename(PackageName));
        Recipe.Manifest->SetNumberField(TEXT("constant_root_yaw_degrees"), Yaw);
        Recipe.Manifest->SetBoolField(TEXT("saved"), false);
        Recipe.Manifest->SetBoolField(TEXT("original_overwritten"), false);
        const int32 Index = Recipes.Num() - 1;
        RecipeBySourceAndTransform.Add(Key, Index);
        return Index;
    };

    // Preflight every source and destination before creating any new package.
    for (FDirectionPlan& Plan : Plans)
    {
        Plan.SourceAO = LoadObject<UBlendSpace>(nullptr, *(SourceRoot + TEXT("AimOffset/") + AONames.FindChecked(Plan.SourceDirection)));
        UAnimSequence* const* CDOIdle = SourceIdleByDirection.Find(Plan.SourceDirection);
        if (!Plan.SourceAO || !CDOIdle || !*CDOIdle) return Fail(TEXT("Missing source AO/CDO Idle for ") + Plan.SourceDirection);
        Plan.Samples = Plan.SourceAO->GetBlendSamples();
        if (Plan.Samples.IsEmpty()) return Fail(TEXT("A source AO has no samples."));
        UAnimSequence* Reference = Plan.Samples[0].Animation ? Plan.Samples[0].Animation->RefPoseSeq.Get() : nullptr;
        if (!Reference) return Fail(TEXT("A source AO has no reference Idle."));
        for (const FBlendSample& Sample : Plan.Samples)
            if (!Sample.Animation || Sample.Animation->RefPoseSeq.Get() != Reference
                || Sample.Animation->AdditiveAnimType != AAT_RotationOffsetMeshSpace)
                return Fail(TEXT("AO samples must share one mesh-space-additive reference: ") + Plan.SourceAO->GetPathName());
        Plan.ReferenceRecipe = AddRecipe(Reference, Plan.TargetDirection, Plan.CenterYaw);
        Plan.IdleRecipe = AddRecipe(*CDOIdle, Plan.TargetDirection, Plan.CenterYaw);
        if (Plan.ReferenceRecipe == INDEX_NONE || Plan.IdleRecipe == INDEX_NONE) return Fail(Error);
        Plan.PackageName = DestinationRoot + Plan.TargetDirection + TEXT("/AO_ProneCanonical_") + Plan.TargetDirection;
        if (PlannedPackages.Contains(Plan.PackageName) || !DestinationAvailable(Plan.PackageName, Error)) return Fail(Error);
        PlannedPackages.Add(Plan.PackageName);
        Plan.Manifest->SetStringField(TEXT("source"), Plan.SourceAO->GetPathName());
        Plan.Manifest->SetStringField(TEXT("direction"), Plan.TargetDirection);
        Plan.Manifest->SetStringField(TEXT("package"), Plan.PackageName);
        Plan.Manifest->SetStringField(TEXT("package_filename"), PackageFilename(Plan.PackageName));
        Plan.Manifest->SetStringField(TEXT("staged_filename"), StagedFilename(Plan.PackageName));
        Plan.Manifest->SetBoolField(TEXT("saved"), false);
        Plan.Manifest->SetBoolField(TEXT("original_overwritten"), false);
    }
    for (FDirectionPlan& Plan : Plans)
        for (const FBlendSample& Sample : Plan.Samples)
        {
            const int32 Recipe = AddRecipe(Sample.Animation, Plan.TargetDirection, -Sample.SampleValue.X);
            if (Recipe == INDEX_NONE) return Fail(Error);
            Plan.SampleRecipes.Add(Recipe);
        }
    FValues Packages;
    for (FSequenceRecipe& Recipe : Recipes) Packages.Add(MakeShared<FJsonValueObject>(Recipe.Manifest));
    for (FDirectionPlan& Plan : Plans) Packages.Add(MakeShared<FJsonValueObject>(Plan.Manifest));
    Result->SetArrayField(TEXT("packages"), Packages);
    Result->SetNumberField(TEXT("sequence_count"), Recipes.Num());
    Result->SetNumberField(TEXT("aim_offset_count"), Plans.Num());
    Result->SetStringField(TEXT("phase"), TEXT("preflight_passed_creating_copies"));

    // Reference/CDO recipes were registered first. Finish these before changing
    // sample tracks so additive compression always sees the canonical base.
    TSet<int32> BaseRecipes;
    for (const FDirectionPlan& Plan : Plans) { BaseRecipes.Add(Plan.ReferenceRecipe); BaseRecipes.Add(Plan.IdleRecipe); }
    for (int32 Index : BaseRecipes)
        if (!TransformSequence(Recipes[Index], nullptr, CommonCenter, Error)) return Fail(Error);
    for (FDirectionPlan& Plan : Plans)
    {
        UAnimSequence* Reference = Recipes[Plan.ReferenceRecipe].Copy;
        for (int32 Index : Plan.SampleRecipes)
            if (!Recipes[Index].Copy && !TransformSequence(Recipes[Index], Reference, CommonCenter, Error)) return Fail(Error);
        NewIdleByDirection.Add(Plan.TargetDirection, Recipes[Plan.IdleRecipe].Copy);
        UPackage* Package = CreatePackage(*Plan.PackageName);
        Plan.CopyAO = DuplicateObject<UBlendSpace>(Plan.SourceAO, Package, *FPackageName::GetLongPackageAssetName(Plan.PackageName));
        if (!Plan.CopyAO) return Fail(TEXT("AO duplication failed: ") + Plan.PackageName);
        Plan.CopyAO->SetFlags(RF_Public | RF_Standalone);
        Plan.CopyAO->Modify();
        // Mirror all sample positions together: sequential EditSampleValue calls
        // would transiently collide with the sample at the opposite yaw.
        FArrayProperty* SampleProperty = FindFProperty<FArrayProperty>(Plan.CopyAO->GetClass(), TEXT("SampleData"));
        if (!SampleProperty || !CastField<FStructProperty>(SampleProperty->Inner)
            || CastField<FStructProperty>(SampleProperty->Inner)->Struct != FBlendSample::StaticStruct())
            return Fail(TEXT("Could not locate the reflected FBlendSample array."));
        TArray<FBlendSample>* Samples = SampleProperty->ContainerPtrToValuePtr<TArray<FBlendSample>>(Plan.CopyAO);
        if (!Samples || Samples->Num() != Plan.Samples.Num()) return Fail(TEXT("Duplicated AO sample count changed."));
        FValues Bindings;
        for (int32 Index = 0; Index < Samples->Num(); ++Index)
        {
            FBlendSample& Sample = (*Samples)[Index];
            Sample.Animation = Recipes[Plan.SampleRecipes[Index]].Copy;
            Sample.SampleValue.X = -Plan.Samples[Index].SampleValue.X;
            FObject Binding = MakeShared<FJsonObject>();
            Binding->SetNumberField(TEXT("source_yaw"), Plan.Samples[Index].SampleValue.X);
            Binding->SetNumberField(TEXT("canonical_yaw"), Sample.SampleValue.X);
            Binding->SetNumberField(TEXT("pitch"), Sample.SampleValue.Y);
            Binding->SetStringField(TEXT("animation"), Sample.Animation->GetPathName());
            Binding->SetStringField(TEXT("reference_idle"), Reference->GetPathName());
            Bindings.Add(MakeShared<FJsonValueObject>(Binding));
        }
        if (FObjectPropertyBase* Preview = FindFProperty<FObjectPropertyBase>(Plan.CopyAO->GetClass(), TEXT("PreviewBasePose")))
            Preview->SetObjectPropertyValue_InContainer(Plan.CopyAO, Reference);
        Plan.CopyAO->ValidateSampleData();
        Plan.CopyAO->ResampleData();
        Plan.CopyAO->MarkPackageDirty();
        Plan.Manifest->SetStringField(TEXT("asset"), Plan.CopyAO->GetPathName());
        Plan.Manifest->SetStringField(TEXT("reference_idle"), Reference->GetPathName());
        Plan.Manifest->SetStringField(TEXT("cdo_idle"), Recipes[Plan.IdleRecipe].Copy->GetPathName());
        Plan.Manifest->SetArrayField(TEXT("sample_bindings"), Bindings);
        NewAOByDirection.Add(Plan.TargetDirection, Plan.CopyAO);
    }
    Result->SetStringField(TEXT("phase"), TEXT("copies_validated_saving_new_packages_to_staging"));
    for (FSequenceRecipe& Recipe : Recipes)
        if (!SaveNewAsset(Recipe.Copy, Recipe.Manifest, Error)) return Fail(Error);
    for (FDirectionPlan& Plan : Plans)
        if (!SaveNewAsset(Plan.CopyAO, Plan.Manifest, Error)) return Fail(Error);
    Result->SetBoolField(TEXT("saved"), true);
    Result->SetStringField(TEXT("phase"), TEXT("new_packages_saved_to_staging"));
    return true;
}
