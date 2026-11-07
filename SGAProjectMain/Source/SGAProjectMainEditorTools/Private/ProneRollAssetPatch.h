#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

class UBlueprint;
class UAnimSequence;
class UBlendSpace;

namespace CharacterAnimationAudit
{
    // Candidate only: stages concrete layer defaults without overwriting Content assets.
    bool StageNaturalProneIdleDefaults(const TMap<FString, UBlueprint*>& Loaded,
        TSharedRef<FJsonObject> Result, TFunctionRef<bool()> WriteCheckpoint);

    // Both maps must contain Forward, Right, Backward and Left.
    bool StageCanonicalProneDefaults(const TMap<FString, UBlueprint*>& Loaded,
        const TMap<FString, UAnimSequence*>& Idle, const TMap<FString, UBlendSpace*>& AO,
        TSharedRef<FJsonObject> Result, TFunctionRef<bool()> WriteCheckpoint);

    // Migrates the exact generated canonical defaults to one shared additive basis.
    bool StageUnifiedProneDefaults(const TMap<FString, UBlueprint*>& Loaded,
        UAnimSequence* Base, UBlendSpace* AO, TSharedRef<FJsonObject> Result,
        TFunctionRef<bool()> WriteCheckpoint);
}
