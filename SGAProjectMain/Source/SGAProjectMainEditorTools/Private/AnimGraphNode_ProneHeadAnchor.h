#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_SkeletalControlBase.h"
#include "../../SGAProjectMain/Character/Animation/AnimNode_ProneHeadAnchor.h"
#include "AnimGraphNode_ProneHeadAnchor.generated.h"

class FBlueprintActionDatabaseRegistrar;

// Kept so historical assets can load and the migration helper can remove it.
// Hidden from creation menus and rejected by the animation Blueprint compiler.
UCLASS()
class UAnimGraphNode_ProneHeadAnchor : public UAnimGraphNode_SkeletalControlBase
{
    GENERATED_BODY()

public:
    UPROPERTY()
    FAnimNode_ProneHeadAnchor Node;

    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    virtual FText GetTooltipText() const override;
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
    virtual void ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog) override;

protected:
    virtual FText GetControllerDescription() const override;
    virtual const FAnimNode_SkeletalControlBase* GetNode() const override { return &Node; }
};
