#pragma once

#include "CoreMinimal.h"

class FJsonObject;
class UAnimSequence;
class UBlendSpace;

// Reads explicit original assets, builds a full-circle AO with one shared base,
// and stages only NEW packages. All four output directions share each pointer.
bool BuildUnifiedProneAssets(TMap<FString, UAnimSequence*>& NewIdleByDirection,
    TMap<FString, UBlendSpace*>& NewAOByDirection, TSharedRef<FJsonObject> Result);
