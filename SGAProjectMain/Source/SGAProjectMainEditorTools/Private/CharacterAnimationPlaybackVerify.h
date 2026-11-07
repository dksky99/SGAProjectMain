#pragma once

#include "CoreMinimal.h"

class FJsonObject;

// Transient EditorPreview evaluation only. Does not begin play, tick actors or save assets.
bool VerifyCharacterAnimationPlayback(TSharedRef<FJsonObject> Result);
bool VerifyCharacterProneRoll(TSharedRef<FJsonObject> Result);
