#pragma once

#include "CoreMinimal.h"

class AHellDiver;
class UCharacterAnimInstance;
class UHellDiverStateComponent;
class USkeletalMeshComponent;
class FJsonObject;

// Evaluates body geometry through public camera input, with no bone constraints.
bool EvaluateCharacterProneRollCases(AHellDiver* Pawn, USkeletalMeshComponent* Mesh,
    UCharacterAnimInstance* Anim, UHellDiverStateComponent* State, TSharedRef<FJsonObject> Result);
