// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Animation/AnimInstance.h"
#include "SentryAnimInstance.generated.h"

UCLASS()
class SGAPROJECTMAIN_API USentryAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	// 센트리에서 최종 Z각을 그대로 넣어줍니다.
	void SetBoneAngles(float rotatorZDeg, float gunHousingZDeg)
	{
		_rotatorZ = rotatorZDeg;
		_gunHousingZ = gunHousingZDeg;
	}

	// 애님 블루프린트에서 읽어가는 값
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	float _rotatorZ = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	float _gunHousingZ = 0.0f;
};