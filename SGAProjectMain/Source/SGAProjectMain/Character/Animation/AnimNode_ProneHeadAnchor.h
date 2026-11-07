#pragma once

#include "CoreMinimal.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_ProneHeadAnchor.generated.h"

// Serialization compatibility for retired assets only. The active MK5 graph
// no longer contains this node. It never alters the incoming pose.
USTRUCT(BlueprintInternalUseOnly)
struct SGAPROJECTMAIN_API FAnimNode_ProneHeadAnchor : public FAnimNode_SkeletalControlBase
{
    GENERATED_BODY()

protected:
    virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output,
        TArray<FBoneTransform>& OutBoneTransforms) override;
    virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
    virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
};
