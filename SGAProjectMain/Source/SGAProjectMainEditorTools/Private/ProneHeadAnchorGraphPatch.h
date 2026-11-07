#pragma once
#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
class UBlueprint;

namespace CharacterAnimationAudit
{
    bool RemoveProneHeadAnchor(UBlueprint* Blueprint, TSharedRef<FJsonObject> Result,
        TFunctionRef<bool()> WriteCheckpoint);
}
