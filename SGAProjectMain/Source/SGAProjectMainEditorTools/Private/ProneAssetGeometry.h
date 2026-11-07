#pragma once

#include "CoreMinimal.h"

class FJsonObject;

// Read-only authoring-data audit. Reconstructs nonadditive component-space poses
// directly from the animation data model, without runtime IK or retargeting.
bool AuditProneAssetGeometry(TSharedRef<FJsonObject> Result);
