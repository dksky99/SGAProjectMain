#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/UnrealType.h"
#include "../Character/HellDiver/HellDiverAnimInstance.h"
#include "../Gun/GunBase.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHelldiverAnimOwnerlessPreviewTest,
    "SGA.Character.AnimationSnapshot.OwnerlessPreview",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelldiverAnimOwnerlessPreviewTest::RunTest(const FString& Parameters)
{
    (void)Parameters;
    USkeletalMeshComponent* PreviewMesh = NewObject<USkeletalMeshComponent>();
    const FVector FirstWorldLocation(125.f, -250.f, 375.f);
    PreviewMesh->SetWorldLocation(FirstWorldLocation);
    UHellDiverAnimInstance* Anim = NewObject<UHellDiverAnimInstance>(PreviewMesh);
    Anim->NativeInitializeAnimation();
    Anim->NativeUpdateAnimation(1.f / 60.f);

    TestFalse(TEXT("A preview without a pawn has no weapon IK target"), Anim->HasValidLeftHandIKTarget());
    TestTrue(TEXT("A preview without a pawn has zero left-hand IK weight"), FMath::IsNearlyZero(Anim->GetLeftHandIKAlpha()));
    TestTrue(TEXT("A mesh without hand_l falls back to its world location"),
        Anim->GetLeftHandIKTargetWorld().Equals(FirstWorldLocation, 0.001f));
    TestFalse(TEXT("The preview fallback is finite"), Anim->GetLeftHandIKTargetWorld().ContainsNaN());

    const FVector NextWorldLocation(-700.f, 850.f, 1000.f);
    PreviewMesh->SetWorldLocation(NextWorldLocation);
    Anim->NativeUpdateAnimation(0.f);
    TestTrue(TEXT("Even a zero-delta update refreshes the world-space fallback"),
        Anim->GetLeftHandIKTargetWorld().Equals(NextWorldLocation, 0.001f));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHelldiverAnimStaleWeaponSnapshotTest,
    "SGA.Character.AnimationSnapshot.StaleWeaponReset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelldiverAnimStaleWeaponSnapshotTest::RunTest(const FString& Parameters)
{
    (void)Parameters;
    USkeletalMeshComponent* PreviewMesh = NewObject<USkeletalMeshComponent>();
    const FVector FallbackWorld(1500.f, -2500.f, 3500.f);
    PreviewMesh->SetWorldLocation(FallbackWorld);
    UHellDiverAnimInstance* Anim = NewObject<UHellDiverAnimInstance>(PreviewMesh);
    Anim->NativeInitializeAnimation();

    FObjectPropertyBase* WeaponProperty = FindFProperty<FObjectPropertyBase>(Anim->GetClass(), TEXT("_curWeapon"));
    FStructProperty* TargetProperty = FindFProperty<FStructProperty>(Anim->GetClass(), TEXT("_leftHandIKTargetWorld"));
    FFloatProperty* AlphaProperty = FindFProperty<FFloatProperty>(Anim->GetClass(), TEXT("_leftHandIKAlpha"));
    FBoolProperty* ValidProperty = FindFProperty<FBoolProperty>(Anim->GetClass(), TEXT("_hasValidLeftHandIKTarget"));
    FBoolProperty* UseHandProperty = FindFProperty<FBoolProperty>(Anim->GetClass(), TEXT("_useLeftHand"));
    FBoolProperty* UseAimProperty = FindFProperty<FBoolProperty>(Anim->GetClass(), TEXT("_bUseAim"));
    if (!TestNotNull(TEXT("The cached weapon property exists"), WeaponProperty)
        || !TestNotNull(TEXT("The cached world target property exists"), TargetProperty)
        || !TestNotNull(TEXT("The cached IK alpha property exists"), AlphaProperty)
        || !TestNotNull(TEXT("The cached IK validity property exists"), ValidProperty)
        || !TestNotNull(TEXT("The compatibility hand flag exists"), UseHandProperty)
        || !TestNotNull(TEXT("The rig aim flag exists"), UseAimProperty))
    {
        return false;
    }

    // Reproduce the state left by a previously equipped weapon before its owner disappears.
    WeaponProperty->SetObjectPropertyValue_InContainer(Anim, GetMutableDefault<AGunBase>());
    *TargetProperty->ContainerPtrToValuePtr<FVector>(Anim) = FVector(99000.f, 98000.f, 97000.f);
    AlphaProperty->SetPropertyValue_InContainer(Anim, 1.f);
    ValidProperty->SetPropertyValue_InContainer(Anim, true);
    UseHandProperty->SetPropertyValue_InContainer(Anim, true);
    Anim->SetWeaponOffset(FTransform(FVector(10.f, 20.f, 30.f)), true);

    Anim->NativeUpdateAnimation(1.f / 60.f);
    TestNull(TEXT("An ownerless update releases the stale weapon reference"),
        WeaponProperty->GetObjectPropertyValue_InContainer(Anim));
    TestFalse(TEXT("The previous weapon target is no longer valid"), Anim->HasValidLeftHandIKTarget());
    TestTrue(TEXT("The previous weapon IK weight is cleared"), FMath::IsNearlyZero(Anim->GetLeftHandIKAlpha()));
    TestTrue(TEXT("The stale world target is replaced by the current preview fallback"),
        Anim->GetLeftHandIKTargetWorld().Equals(FallbackWorld, 0.001f));
    TestFalse(TEXT("Legacy hand consumers cannot retain the old grip"), UseHandProperty->GetPropertyValue_InContainer(Anim));
    TestFalse(TEXT("The previous weapon cannot leave rig aiming enabled"), UseAimProperty->GetPropertyValue_InContainer(Anim));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHelldiverAnimEquipChangeTest,
    "SGA.Character.AnimationSnapshot.EquipChangePulse",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelldiverAnimEquipChangeTest::RunTest(const FString& Parameters)
{
    (void)Parameters;
    USkeletalMeshComponent* PreviewMesh = NewObject<USkeletalMeshComponent>();
    UHellDiverAnimInstance* Anim = NewObject<UHellDiverAnimInstance>(PreviewMesh);
    Anim->NativeInitializeAnimation();

    Anim->CheckEquipChange(1);
    TestTrue(TEXT("The first change to slot 1 emits a change pulse"), Anim->_changeWeapon);
    TestEqual(TEXT("The first change stores slot 1"), Anim->_curEquipIndex, static_cast<uint8>(1));
    Anim->CheckEquipChange(1);
    TestFalse(TEXT("The same slot on the next frame does not repeat the pulse"), Anim->_changeWeapon);
    Anim->CheckEquipChange(2);
    TestTrue(TEXT("Changing to slot 2 emits a new pulse"), Anim->_changeWeapon);
    TestEqual(TEXT("The new slot is retained for the following frame"), Anim->_curEquipIndex, static_cast<uint8>(2));
    return true;
}

#endif
