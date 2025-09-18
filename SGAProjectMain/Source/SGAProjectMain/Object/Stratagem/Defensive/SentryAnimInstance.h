// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SentryAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API USentryAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	USentryAnimInstance();

protected:
	// 애님 초기화
	virtual void NativeInitializeAnimation() override;

	// 매 프레임 갱신 (여기서는 추가 보간/필터가 필요할 때만 사용)
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	// 센트리에서 계산된 조준 각도를 전달
	UFUNCTION(BlueprintCallable, Category = "Game/Stratagem/Sentry")
	void SetAimAngles(float yawDeg, float pitchDeg);

	// 조준 레이어 강도(0.0~1.0). AnimGraph에서 Layered Blend Per Bone의 알파로 사용
	UFUNCTION(BlueprintCallable, Category = "Game/Stratagem/Sentry")
	void SetAimAlpha(float alpha);

	// 블루프린트에서 읽기용
	UFUNCTION(BlueprintPure, Category = "Game/Stratagem/Sentry")
	float GetAimYawDeg() const { return _aimYawDeg; }

	UFUNCTION(BlueprintPure, Category = "Game/Stratagem/Sentry")
	float GetAimPitchDeg() const { return _aimPitchDeg; }

	UFUNCTION(BlueprintPure, Category = "Game/Stratagem/Sentry")
	float GetAimAlpha() const { return _aimAlpha; }

	// -------------------------------------------------------
	// 변수 (Aim State)
protected:
	// 센트리에서 전달된 최종 조준 각도(도)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	float _aimYawDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	float _aimPitchDeg = 0.0f;

	// 조준 레이어 적용 강도(0.0~1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _aimAlpha = 1.0f;

	// 선택: 애님단 추가 보간(센트리에서 이미 보간 중이면 0.0f로 두세요)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _extraSmoothingSpeed = 0.0f;
};
