#pragma once
#include "AnimGraphNode_SkeletalControlBase.h"
#include "../../SGAProjectMain/Character/Animation/AnimNode_UpperBodyAim.h"
#include "AnimGraphNode_UpperBodyAim.generated.h"
UCLASS()
class UAnimGraphNode_UpperBodyAim : public UAnimGraphNode_SkeletalControlBase
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category=Settings) FAnimNode_UpperBodyAim Node;
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override { return FText::FromString(TEXT("Distributed Upper Body Aim")); }
    virtual FText GetTooltipText() const override { return FText::FromString(TEXT("Bounded spine aiming and same-frame muzzle correction before left hand IK.")); }
protected:
    virtual const FAnimNode_SkeletalControlBase* GetNode() const override { return &Node; }
};
