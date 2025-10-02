// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define ECC_GameDamage ECC_GameTraceChannel2

UENUM(BlueprintType)
enum class ETeamID : uint8
{
	HellDiver = 0 UMETA(DisplayName = "HellDiver"),
	Enemy = 1 UMETA(DisplayName = "Far"),
	MAX
};
