// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Animation/AnimInstance.h"
#include "SentryAnimInstance.generated.h"

UCLASS()
class SGAPROJECTMAIN_API USentryAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	void SetLookAtYawTargetWS(const FVector& v) { _lookAtTargetYawWS = v; }
	void SetPitchDeg(float v) { _aimPitchDeg = v; }

public:
	// LookAt¿ë ¿ùµå ÁÂÇ¥ Å¸±ê(¾ß¿À/ÇÇÄ¡)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	FVector _lookAtTargetYawWS = FVector::ZeroVector;

	// Transform¿ë ÁÂÇ¥
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	float _aimPitchDeg = 0.0f;
};