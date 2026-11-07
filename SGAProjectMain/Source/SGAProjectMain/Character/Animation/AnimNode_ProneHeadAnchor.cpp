#include "AnimNode_ProneHeadAnchor.h"

void FAnimNode_ProneHeadAnchor::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext&,
    TArray<FBoneTransform>&)
{
}

bool FAnimNode_ProneHeadAnchor::IsValidToEvaluate(const USkeleton*, const FBoneContainer&)
{
    return false;
}

void FAnimNode_ProneHeadAnchor::InitializeBoneReferences(const FBoneContainer&)
{
}
