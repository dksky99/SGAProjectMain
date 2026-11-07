#pragma once
#include "CoreMinimal.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_UpperBodyAim.generated.h"

// Final component-space correction after root rotation, Control Rig and foot/pelvis placement.
USTRUCT(BlueprintInternalUseOnly)
struct SGAPROJECTMAIN_API FAnimNode_UpperBodyAim : public FAnimNode_SkeletalControlBase
{
    GENERATED_BODY()
    FAnimNode_UpperBodyAim();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Inputs, meta=(PinShownByDefault))
    FVector TargetPosition = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Inputs, meta=(PinShownByDefault))
    FTransform MuzzleInHand = FTransform::Identity;
    // IK_left position relative to hand_r. Socket orientation is not a wrist contract.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Inputs, meta=(PinShownByDefault))
    FTransform LeftGripInHand = FTransform::Identity;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Inputs, meta=(PinShownByDefault, ClampMin="0", ClampMax="1"))
    float LeftHandAlpha = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Inputs, meta=(PinShownByDefault))
    FVector BodyForward = FVector(0,1,0);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Inputs, meta=(PinShownByDefault))
    FVector BodyUp = FVector::UpVector;
    UPROPERTY(EditAnywhere, Category=Torso, meta=(ClampMin="0",ClampMax="60"))
    float MaxTorsoYaw = 45.f;
    UPROPERTY(EditAnywhere, Category=Torso, meta=(ClampMin="0",ClampMax="45"))
    float MaxTorsoPitch = 30.f;
    UPROPERTY(EditAnywhere, Category=Bones) FBoneReference Root;
    UPROPERTY(EditAnywhere, Category=Bones) FBoneReference Spine1;
    UPROPERTY(EditAnywhere, Category=Bones) FBoneReference Spine2;
    UPROPERTY(EditAnywhere, Category=Bones) FBoneReference Spine3;
    UPROPERTY(EditAnywhere, Category=Bones) FBoneReference UpperArm;
    UPROPERTY(EditAnywhere, Category=Bones) FBoneReference Hand;
    UPROPERTY(EditAnywhere, Category=Bones) FBoneReference LeftUpperArm;
    UPROPERTY(EditAnywhere, Category=Bones) FBoneReference LeftLowerArm;
    UPROPERTY(EditAnywhere, Category=Bones) FBoneReference LeftHand;
protected:
    virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
    virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
    virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
};
