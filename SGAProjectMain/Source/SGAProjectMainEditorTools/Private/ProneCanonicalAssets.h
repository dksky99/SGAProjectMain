#pragma once

#include "CoreMinimal.h"

class FJsonObject;
class UAnimSequence;
class UBlendSpace;

// Source keys are the current CDO's Forward/Right/Backward/Left directions.
// Creates separate, canonical authoring assets in memory and saves their NEW
// packages only beneath Saved/CharacterAnimationAudit/CanonicalStaged.
bool BuildProneCanonicalAssets(const TMap<FString, UAnimSequence*>& SourceIdleByDirection,
    TMap<FString, UAnimSequence*>& NewIdleByDirection,
    TMap<FString, UBlendSpace*>& NewAOByDirection,
    TSharedRef<FJsonObject> Result);
