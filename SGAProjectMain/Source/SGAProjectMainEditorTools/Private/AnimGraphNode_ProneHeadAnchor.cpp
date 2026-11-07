#include "AnimGraphNode_ProneHeadAnchor.h"
#include "Kismet2/CompilerResultsLog.h"

#define LOCTEXT_NAMESPACE "SGAProneHeadAnchor"

FText UAnimGraphNode_ProneHeadAnchor::GetNodeTitle(ENodeTitleType::Type) const
{
    return GetControllerDescription();
}

FText UAnimGraphNode_ProneHeadAnchor::GetControllerDescription() const
{
    return LOCTEXT("RetiredTitle", "Retired Prone Head Anchor (Pass Through)");
}

FText UAnimGraphNode_ProneHeadAnchor::GetTooltipText() const
{
    return LOCTEXT("RetiredTooltip", "Retired compatibility node. It never modifies the pose. Remove it and connect the component-space pose directly.");
}

void UAnimGraphNode_ProneHeadAnchor::GetMenuActions(FBlueprintActionDatabaseRegistrar&) const
{
}

void UAnimGraphNode_ProneHeadAnchor::ValidateAnimNodeDuringCompilation(USkeleton*, FCompilerResultsLog& MessageLog)
{
    MessageLog.Error(TEXT("@@ is retired. Remove this pass-through compatibility node; head pinning does not implement prone roll."), this);
}

#undef LOCTEXT_NAMESPACE
