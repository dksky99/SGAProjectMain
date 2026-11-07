#include "ProneUnifiedAssets.h"

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

namespace ProneUnifiedAssets
{
    using FObject = TSharedRef<FJsonObject>;
    using FValues = TArray<TSharedPtr<FJsonValue>>;
    const FString SourceRoot = TEXT("/Game/Blueprints/HellDiver/Animation_Locomotion/");
    const FString DestinationRoot = SourceRoot + TEXT("ProneUnified/");

    FObject Vector(const FVector& V)
    {
        FObject Out = MakeShared<FJsonObject>();
        Out->SetNumberField(TEXT("x"), V.X); Out->SetNumberField(TEXT("y"), V.Y); Out->SetNumberField(TEXT("z"), V.Z);
        return Out;
    }

    FString PackageFilename(const FString& Package)
    {
        return FPaths::ConvertRelativePathToFull(FPackageName::LongPackageNameToFilename(Package, FPackageName::GetAssetPackageExtension()));
    }

    FString StagedFilename(const FString& Package)
    {
        check(Package.StartsWith(TEXT("/Game/")));
        return FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectSavedDir(),
            TEXT("CharacterAnimationAudit/UnifiedStaged"), Package.Mid(6) + TEXT(".uasset")));
    }

    int32 RotationYaw(int32 Yaw) { return FMath::Abs(Yaw) == 180 ? 180 : Yaw; }
    FString AngleName(int32 Angle) { return FString::Printf(TEXT("%c%03d"), Angle < 0 ? TCHAR('M') : TCHAR('P'), FMath::Abs(Angle)); }

    bool RawPelvis(UAnimSequence* Sequence, FVector& Position, FString& Error)
    {
        const IAnimationDataModel* Model = Sequence ? Sequence->GetDataModel() : nullptr;
        const USkeleton* Skeleton = Sequence ? Sequence->GetSkeleton() : nullptr;
        if (!Model || !Skeleton || Model->GetNumberOfKeys() == 0)
        { Error = TEXT("A source has no raw model/skeleton/keys: ") + GetPathNameSafe(Sequence); return false; }
        const FReferenceSkeleton& Ref = Skeleton->GetReferenceSkeleton();
        const int32 Pelvis = Ref.FindBoneIndex(TEXT("pelvis"));
        if (Pelvis == INDEX_NONE) { Error = TEXT("The source skeleton has no pelvis."); return false; }
        TArray<FTransform> CS; CS.SetNum(Ref.GetNum());
        for (int32 Bone = 0; Bone < Ref.GetNum(); ++Bone)
        {
            const FName Name = Ref.GetBoneName(Bone);
            const FTransform Local = Model->IsValidBoneTrackName(Name)
                ? Model->GetBoneTrackTransform(Name, FFrameNumber(0)) : Ref.GetRefBonePose()[Bone];
            const int32 Parent = Ref.GetParentIndex(Bone);
            CS[Bone] = Parent == INDEX_NONE ? Local : Local * CS[Parent];
            CS[Bone].NormalizeRotation();
            if (CS[Bone].ContainsNaN()) { Error = TEXT("A raw bone transform is not finite."); return false; }
        }
        Position = CS[Pelvis].GetTranslation();
        return true;
    }

    bool SameLocalTracks(UAnimSequence* A, UAnimSequence* B, bool bIncludeRoot, FString& Error)
    {
        const IAnimationDataModel* AM = A ? A->GetDataModel() : nullptr;
        const IAnimationDataModel* BM = B ? B->GetDataModel() : nullptr;
        if (!AM || !BM || A->GetSkeleton() != B->GetSkeleton()
            || AM->GetNumberOfKeys() != BM->GetNumberOfKeys() || AM->GetFrameRate() != BM->GetFrameRate())
        { Error = TEXT("Animation model/skeleton/timebase differs."); return false; }
        const FReferenceSkeleton& Ref = A->GetSkeleton()->GetReferenceSkeleton();
        for (int32 Bone = bIncludeRoot ? 0 : 1; Bone < Ref.GetNum(); ++Bone)
        {
            const FName Name = Ref.GetBoneName(Bone);
            const bool bA = AM->IsValidBoneTrackName(Name), bB = BM->IsValidBoneTrackName(Name);
            if (bA != bB) { Error = TEXT("Track binding differs: ") + Name.ToString(); return false; }
            if (!bA) continue;
            TArray<FTransform> Before, After;
            AM->GetBoneTrackTransforms(Name, Before); BM->GetBoneTrackTransforms(Name, After);
            if (Before.Num() != After.Num()) { Error = TEXT("Track key count differs: ") + Name.ToString(); return false; }
            for (int32 Key = 0; Key < Before.Num(); ++Key)
                if (!Before[Key].Equals(After[Key], 0.0001))
                { Error = FString::Printf(TEXT("Raw local key differs: %s frame %d"), *Name.ToString(), Key); return false; }
        }
        return true;
    }

    struct FRecipe
    {
        UAnimSequence* Source = nullptr;
        UAnimSequence* Copy = nullptr;
        int32 Yaw = 0;
        FString Package;
        FObject Manifest = MakeShared<FJsonObject>();
    };

    bool Available(const FString& Package, FString& Error)
    {
        if (FindPackage(nullptr, *Package) || FPackageName::DoesPackageExist(Package)
            || IFileManager::Get().FileExists(*PackageFilename(Package))
            || IFileManager::Get().FileExists(*StagedFilename(Package)))
        { Error = TEXT("Refusing to overwrite an existing unified or staged package: ") + Package; return false; }
        return true;
    }

    void ManifestPaths(FObject Manifest, UObject* Source, const FString& Package)
    {
        Manifest->SetStringField(TEXT("source"), GetPathNameSafe(Source));
        Manifest->SetStringField(TEXT("package"), Package);
        Manifest->SetStringField(TEXT("package_filename"), PackageFilename(Package));
        Manifest->SetStringField(TEXT("staged_filename"), StagedFilename(Package));
        Manifest->SetBoolField(TEXT("saved"), false);
        Manifest->SetBoolField(TEXT("original_overwritten"), false);
    }

    bool CopySequence(FRecipe& Recipe, UAnimSequence* SharedReference, const FVector& CommonCenter, FString& Error)
    {
        FVector SourcePelvis;
        if (!RawPelvis(Recipe.Source, SourcePelvis, Error)) return false;
        const FVector Pivot(SourcePelvis.X, SourcePelvis.Y, 0.0), Target(CommonCenter.X, CommonCenter.Y, 0.0);
        const FQuat Correction = FRotator(0.0, Recipe.Yaw, 0.0).Quaternion();
        Recipe.Source->WaitOnExistingCompression();
        UPackage* Package = CreatePackage(*Recipe.Package);
        if (!Package) { Error = TEXT("CreatePackage failed."); return false; }
        Recipe.Copy = DuplicateObject<UAnimSequence>(Recipe.Source, Package, *FPackageName::GetLongPackageAssetName(Recipe.Package));
        UAnimSequence* Copy = Recipe.Copy;
        if (!Copy || Copy->GetDataModel() == Recipe.Source->GetDataModel())
        { Error = TEXT("Animation duplicate has no independent data model."); return false; }
        Copy->SetFlags(RF_Public | RF_Standalone); Copy->Modify();
        if (SharedReference) Copy->RefPoseSeq = SharedReference;
        const IAnimationDataModel* Model = Copy->GetDataModel();
        const FName Root = Copy->GetSkeleton()->GetReferenceSkeleton().GetBoneName(0);
        if (!Model->IsValidBoneTrackName(Root)) { Error = TEXT("A source animation has no root track."); return false; }
        TArray<FTransform> Keys; Model->GetBoneTrackTransforms(Root, Keys);
        if (Keys.Num() != Model->GetNumberOfKeys()) { Error = TEXT("Root key count differs from the animation model."); return false; }
        TArray<FVector> Positions, Scales; TArray<FQuat> Rotations;
        Positions.Reserve(Keys.Num()); Scales.Reserve(Keys.Num()); Rotations.Reserve(Keys.Num());
        for (const FTransform& Key : Keys)
        {
            const FVector Position = Correction.RotateVector(Key.GetTranslation() - Pivot) + Target;
            FQuat Rotation = Correction * Key.GetRotation(); Rotation.Normalize();
            if (Position.ContainsNaN() || Rotation.ContainsNaN()) { Error = TEXT("Non-finite canonical root key."); return false; }
            Positions.Add(Position); Rotations.Add(Rotation); Scales.Add(Key.GetScale3D());
        }
        IAnimationDataController& Controller = Copy->GetController();
        Controller.OpenBracket(FText::FromString(TEXT("Bake constant unified prone authoring transform")), false);
        const bool bSet = Controller.SetBoneTrackKeys(Root, Positions, Rotations, Scales, false);
        Controller.CloseBracket(false);
        if (!bSet) { Error = TEXT("SetBoneTrackKeys failed."); return false; }
        Copy->CacheDerivedDataForCurrentPlatform(); Copy->WaitOnExistingCompression(); Copy->MarkPackageDirty();
        FVector After;
        if (!RawPelvis(Copy, After, Error)) return false;
        if (!FVector2D(After.X, After.Y).Equals(FVector2D(CommonCenter.X, CommonCenter.Y), 0.01)
            || !FMath::IsNearlyEqual(After.Z, SourcePelvis.Z, 0.01))
        { Error = TEXT("Representative pelvis XY / unchanged Z validation failed."); return false; }
        if (!SameLocalTracks(Recipe.Source, Copy, false, Error)) return false;
        if (Copy->AdditiveAnimType != Recipe.Source->AdditiveAnimType || Copy->RefPoseType != Recipe.Source->RefPoseType
            || Copy->RefFrameIndex != Recipe.Source->RefFrameIndex)
        { Error = TEXT("The source additive contract changed unexpectedly."); return false; }
        Recipe.Manifest->SetStringField(TEXT("asset"), Copy->GetPathName());
        Recipe.Manifest->SetNumberField(TEXT("constant_root_yaw_degrees"), Recipe.Yaw);
        Recipe.Manifest->SetObjectField(TEXT("source_pelvis_frame0_component"), Vector(SourcePelvis));
        Recipe.Manifest->SetObjectField(TEXT("unified_pelvis_frame0_component"), Vector(After));
        Recipe.Manifest->SetNumberField(TEXT("root_key_count"), Keys.Num());
        Recipe.Manifest->SetBoolField(TEXT("non_root_local_keys_unchanged"), true);
        Recipe.Manifest->SetBoolField(TEXT("derived_data_cached_synchronously"), true);
        Recipe.Manifest->SetStringField(TEXT("ref_pose_sequence"), GetPathNameSafe(Copy->RefPoseSeq.Get()));
        Recipe.Manifest->SetNumberField(TEXT("additive_anim_type"), static_cast<int32>(Copy->AdditiveAnimType.GetValue()));
        return true;
    }

    bool Save(UObject* Asset, FObject Manifest, FString& Error)
    {
        if (!Asset || !Asset->GetOutermost()->GetName().StartsWith(DestinationRoot))
        { Error = TEXT("Only new ProneUnified packages can be staged."); return false; }
        const FString Filename = StagedFilename(Asset->GetOutermost()->GetName());
        if (IFileManager::Get().FileExists(*Filename)) { Error = TEXT("An existing staged file would be overwritten."); return false; }
        if (!IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true))
        { Error = TEXT("Could not create unified staging directory."); return false; }
        FSavePackageArgs Args; Args.TopLevelFlags = RF_Public | RF_Standalone;
        Args.SaveFlags = SAVE_NoError; Args.Error = GWarn; Args.bSlowTask = false;
        const bool bSaved = UPackage::SavePackage(Asset->GetOutermost(), Asset, *Filename, Args);
        Manifest->SetBoolField(TEXT("saved"), bSaved); Manifest->SetBoolField(TEXT("saved_to_staging"), bSaved);
        if (!bSaved) Error = TEXT("Unified SavePackage failed: ") + Filename;
        return bSaved;
    }
}

bool BuildUnifiedProneAssets(TMap<FString, UAnimSequence*>& NewIdleByDirection,
    TMap<FString, UBlendSpace*>& NewAOByDirection, TSharedRef<FJsonObject> Result)
{
    using namespace ProneUnifiedAssets;
    NewIdleByDirection.Reset(); NewAOByDirection.Reset();
    Result->SetBoolField(TEXT("saved"), false);
    Result->SetBoolField(TEXT("source_packages_saved_or_overwritten"), false);
    Result->SetStringField(TEXT("destination_root"), DestinationRoot);
    Result->SetStringField(TEXT("scope"), TEXT("One full-circle AO and one shared base for all four directions. Constant per-clip root authoring transform only; no runtime bone constraints. Original source files are not saved."));
    FString Error;
    const auto Fail = [&Result](const FString& Message)
    {
        Result->SetStringField(TEXT("error"), Message); Result->SetStringField(TEXT("phase"), TEXT("failed"));
        UE_LOG(LogTemp, Error, TEXT("ProneUnifiedAssets: %s"), *Message); return false;
    };

    TArray<UBlendSpace*> SourceAOs;
    for (const TCHAR* Name : { TEXT("AO_ProneFwd"), TEXT("AO_ProneRight"), TEXT("AO_ProneBwd"), TEXT("AO_ProneLeft") })
    {
        UBlendSpace* AO = LoadObject<UBlendSpace>(nullptr, *(SourceRoot + TEXT("AimOffset/") + Name));
        if (!AO || AO->GetBlendSamples().IsEmpty()) return Fail(TEXT("An explicit original source AO could not be loaded."));
        SourceAOs.Add(AO);
    }
    UAnimSequence* ReferenceSource = SourceAOs[0]->GetBlendSamples()[0].Animation
        ? SourceAOs[0]->GetBlendSamples()[0].Animation->RefPoseSeq.Get() : nullptr;
    UAnimSequence* CenterSource = LoadObject<UAnimSequence>(nullptr,
        *(SourceRoot + TEXT("Assets/Prone/Helldiver_Anim_Prone_Pistol_Idle_R")));
    FVector CommonCenter;
    if (!ReferenceSource || ReferenceSource->AdditiveAnimType != AAT_None || !RawPelvis(CenterSource, CommonCenter, Error))
        return Fail(Error.IsEmpty() ? TEXT("The original common reference/center source is unavailable.") : Error);
    Result->SetStringField(TEXT("reference_source"), ReferenceSource->GetPathName());
    Result->SetStringField(TEXT("common_center_source"), CenterSource->GetPathName());
    Result->SetObjectField(TEXT("common_frame0_pelvis_component"), Vector(CommonCenter));

    TMap<FIntPoint, FBlendSample> Union;
    FValues MergedOverlaps;
    for (UBlendSpace* AO : SourceAOs)
        for (const FBlendSample& Sample : AO->GetBlendSamples())
        {
            const int32 X = FMath::RoundToInt(Sample.SampleValue.X), Pitch = FMath::RoundToInt(Sample.SampleValue.Y);
            if (!Sample.Animation || Sample.Animation->AdditiveAnimType != AAT_RotationOffsetMeshSpace
                || !FMath::IsNearlyEqual(Sample.SampleValue.X, double(X), 0.001)
                || !FMath::IsNearlyEqual(Sample.SampleValue.Y, double(Pitch), 0.001)
                || FMath::Abs(X) > 180 || X % 45 != 0 || FMath::Abs(Pitch) > 90 || Pitch % 90 != 0)
                return Fail(TEXT("An original AO sample has an unsupported yaw/pitch/additive contract."));
            const FIntPoint Coordinate(X, Pitch);
            if (FBlendSample* Existing = Union.Find(Coordinate))
            {
                if (!SameLocalTracks(Existing->Animation, Sample.Animation, true, Error)
                    || !FMath::IsNearlyEqual(Existing->RateScale, Sample.RateScale))
                    return Fail(TEXT("Overlapping original AO samples are not equivalent: ") + Error);
                FObject Merge = MakeShared<FJsonObject>();
                Merge->SetNumberField(TEXT("source_yaw"), X); Merge->SetNumberField(TEXT("pitch"), Pitch);
                Merge->SetStringField(TEXT("kept"), Existing->Animation->GetPathName());
                Merge->SetStringField(TEXT("equivalent_source"), Sample.Animation->GetPathName());
                Merge->SetBoolField(TEXT("all_raw_local_keys_equivalent"), true);
                MergedOverlaps.Add(MakeShared<FJsonValueObject>(Merge));
            }
            else Union.Add(Coordinate, Sample);
        }
    Result->SetArrayField(TEXT("merged_overlapping_source_samples"), MergedOverlaps);
    if (Union.Num() != 27) return Fail(TEXT("Expected nine yaw coordinates times three pitch coordinates."));
    for (int32 X = -180; X <= 180; X += 45)
        for (int32 Pitch = -90; Pitch <= 90; Pitch += 90)
            if (!Union.Contains(FIntPoint(X, Pitch))) return Fail(TEXT("The full-circle AO union has a missing sample coordinate."));

    TArray<FRecipe> Recipes;
    FRecipe& Base = Recipes.AddDefaulted_GetRef();
    Base.Source = ReferenceSource; Base.Package = DestinationRoot + TEXT("ProneUnified_Base");
    ManifestPaths(Base.Manifest, Base.Source, Base.Package);
    TMap<FIntPoint, int32> RecipeForCanonicalCoordinate;
    TArray<FIntPoint> SourceCoordinates;
    Union.GetKeys(SourceCoordinates);
    SourceCoordinates.Sort([](const FIntPoint& A, const FIntPoint& B) { return A.X == B.X ? A.Y < B.Y : A.X < B.X; });
    for (const FIntPoint& Coordinate : SourceCoordinates)
    {
        const FIntPoint Canonical(RotationYaw(-Coordinate.X), Coordinate.Y);
        UAnimSequence* Source = Union.FindChecked(Coordinate).Animation;
        if (const int32* Existing = RecipeForCanonicalCoordinate.Find(Canonical))
        {
            if (!SameLocalTracks(Recipes[*Existing].Source, Source, true, Error))
                return Fail(TEXT("The +/-180 seam poses differ: ") + Error);
            continue;
        }
        FRecipe& Recipe = Recipes.AddDefaulted_GetRef();
        Recipe.Source = Source; Recipe.Yaw = Canonical.X;
        Recipe.Package = DestinationRoot + TEXT("ProneUnified_Yaw_") + AngleName(Canonical.X)
            + TEXT("_Pitch_") + AngleName(Canonical.Y);
        ManifestPaths(Recipe.Manifest, Source, Recipe.Package);
        RecipeForCanonicalCoordinate.Add(Canonical, Recipes.Num() - 1);
    }
    const FString AOPackage = DestinationRoot + TEXT("AO_ProneUnified");
    FObject AOManifest = MakeShared<FJsonObject>(); ManifestPaths(AOManifest, SourceAOs[0], AOPackage);
    for (const FRecipe& Recipe : Recipes) if (!Available(Recipe.Package, Error)) return Fail(Error);
    if (!Available(AOPackage, Error)) return Fail(Error);
    FValues Packages;
    for (FRecipe& Recipe : Recipes) Packages.Add(MakeShared<FJsonValueObject>(Recipe.Manifest));
    Packages.Add(MakeShared<FJsonValueObject>(AOManifest));
    Result->SetArrayField(TEXT("packages"), Packages);
    Result->SetNumberField(TEXT("sequence_count"), Recipes.Num());
    Result->SetNumberField(TEXT("unique_sample_sequence_count"), Recipes.Num() - 1);
    Result->SetNumberField(TEXT("aim_offset_count"), 1);
    Result->SetNumberField(TEXT("sample_slot_count"), Union.Num());
    Result->SetStringField(TEXT("phase"), TEXT("preflight_passed_creating_copies"));
    if (!CopySequence(Recipes[0], nullptr, CommonCenter, Error)) return Fail(Error);
    UAnimSequence* SharedBase = Recipes[0].Copy;
    for (int32 Index = 1; Index < Recipes.Num(); ++Index)
        if (!CopySequence(Recipes[Index], SharedBase, CommonCenter, Error)) return Fail(Error);

    UPackage* Package = CreatePackage(*AOPackage);
    UBlendSpace* UnifiedAO = Package ? DuplicateObject<UBlendSpace>(SourceAOs[0], Package, TEXT("AO_ProneUnified")) : nullptr;
    if (!UnifiedAO) return Fail(TEXT("Could not duplicate the unified AO."));
    UnifiedAO->SetFlags(RF_Public | RF_Standalone); UnifiedAO->Modify();
    FArrayProperty* SampleProperty = FindFProperty<FArrayProperty>(UnifiedAO->GetClass(), TEXT("SampleData"));
    FStructProperty* Inner = SampleProperty ? CastField<FStructProperty>(SampleProperty->Inner) : nullptr;
    FObjectPropertyBase* PreviewProperty = FindFProperty<FObjectPropertyBase>(UnifiedAO->GetClass(), TEXT("PreviewBasePose"));
    if (!Inner || Inner->Struct != FBlendSample::StaticStruct() || !PreviewProperty)
        return Fail(TEXT("The unified AO's sample/preview reflected properties are unavailable."));
    TArray<FBlendSample>* Samples = SampleProperty->ContainerPtrToValuePtr<TArray<FBlendSample>>(UnifiedAO);
    Samples->Reset(27);
    FValues Bindings;
    for (const FIntPoint& Coordinate : SourceCoordinates)
    {
        FBlendSample Sample = Union.FindChecked(Coordinate);
        const FIntPoint RecipeCoordinate(RotationYaw(-Coordinate.X), Coordinate.Y);
        Sample.Animation = Recipes[RecipeForCanonicalCoordinate.FindChecked(RecipeCoordinate)].Copy;
        Sample.SampleValue.X = -Coordinate.X;
        Samples->Add(Sample);
        FObject Binding = MakeShared<FJsonObject>();
        Binding->SetNumberField(TEXT("source_yaw"), Coordinate.X);
        Binding->SetNumberField(TEXT("canonical_yaw"), Sample.SampleValue.X);
        Binding->SetNumberField(TEXT("pitch"), Coordinate.Y);
        Binding->SetStringField(TEXT("animation"), Sample.Animation->GetPathName());
        Binding->SetStringField(TEXT("reference_idle"), SharedBase->GetPathName());
        Bindings.Add(MakeShared<FJsonValueObject>(Binding));
    }
    PreviewProperty->SetObjectPropertyValue_InContainer(UnifiedAO, SharedBase);
    UnifiedAO->ValidateSampleData(); UnifiedAO->ResampleData(); UnifiedAO->MarkPackageDirty();
    if (UnifiedAO->GetBlendSamples().Num() != 27) return Fail(TEXT("AO validation changed the required 27 sample slots."));
    for (const FBlendSample& Sample : UnifiedAO->GetBlendSamples())
        if (!Sample.Animation || Sample.Animation->RefPoseSeq.Get() != SharedBase || !Sample.bIsValid)
            return Fail(TEXT("An AO sample is invalid or no longer uses the shared reference."));
    AOManifest->SetStringField(TEXT("asset"), UnifiedAO->GetPathName());
    AOManifest->SetStringField(TEXT("reference_idle"), SharedBase->GetPathName());
    AOManifest->SetStringField(TEXT("preview_base_pose"), SharedBase->GetPathName());
    AOManifest->SetArrayField(TEXT("sample_bindings"), Bindings);
    Result->SetStringField(TEXT("shared_idle"), SharedBase->GetPathName());
    Result->SetStringField(TEXT("shared_ao"), UnifiedAO->GetPathName());
    Result->SetBoolField(TEXT("all_directions_share_identical_base_and_ao"), true);
    for (const TCHAR* Direction : { TEXT("Forward"), TEXT("Right"), TEXT("Backward"), TEXT("Left") })
    { NewIdleByDirection.Add(Direction, SharedBase); NewAOByDirection.Add(Direction, UnifiedAO); }
    Result->SetStringField(TEXT("phase"), TEXT("validated_saving_new_packages_to_staging"));
    for (FRecipe& Recipe : Recipes) if (!Save(Recipe.Copy, Recipe.Manifest, Error)) return Fail(Error);
    if (!Save(UnifiedAO, AOManifest, Error)) return Fail(Error);
    Result->SetBoolField(TEXT("saved"), true);
    Result->SetStringField(TEXT("phase"), TEXT("new_packages_saved_to_staging"));
    return true;
}
