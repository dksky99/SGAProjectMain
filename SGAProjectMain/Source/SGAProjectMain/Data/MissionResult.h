// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCurrency.h"
#include "MissionResult.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class ERewardCategory : uint8
{
	MainObjective,
	OptionalObjectives,
	HelldiversExtracted,
	//OutpostsDestroyed,
	MissionTimeRemaining
};

USTRUCT()
struct FMissionReward
{
	GENERATED_BODY()

	int32 _experience = 0;
	int32 _requisitionSlips = 0;

	ERewardCategory _category = ERewardCategory::MainObjective;
};

USTRUCT()
struct FMissionResult
{
	GENERATED_BODY()

	UPROPERTY()
	class UOperationDataAsset* _operation = nullptr;
	UPROPERTY()
	class UMissionDataAsset* _mission = nullptr;

	UPROPERTY()
	TSet<FName> _completedOptionalObjectives;

	bool _isMissionCleared = false; // 메인 목표 클리어 여부
	int32 _extractedHelldiversNum = 0; // 탈출한 헬다이버 수
	int32 _clearedMissionNum = 0; // 해당 임무에서 지금까지 클리어한 미션 수
	float _remainingTimeRatio = 1.f; // 남은 시간 비율

	UPROPERTY()
	TArray<FMissionReward> _missionRewards;
	UPROPERTY()
	FPlayerCurrency _totalReward;
};