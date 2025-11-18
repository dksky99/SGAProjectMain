// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
// 트레이스 채널(레이저/라인트레이스/LOS에서 사용)
#define ECC_GameDamage ECC_GameTraceChannel2

// 오브젝트 채널(컴포넌트의 Object Type 비교용)
static constexpr ECollisionChannel OBJECT_PROJECTILE = ECollisionChannel::ECC_GameTraceChannel1;
static constexpr ECollisionChannel OBJECT_GAMEDAMAGEBODY = ECollisionChannel::ECC_GameTraceChannel2;

UENUM(BlueprintType)
enum class ETeamID : uint8
{
	HellDiver = 0 UMETA(DisplayName = "HellDiver"),
	Enemy = 1 UMETA(DisplayName = "Enemy"),
	MAX
};
