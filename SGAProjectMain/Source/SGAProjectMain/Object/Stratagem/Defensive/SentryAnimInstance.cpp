// Fill out your copyright notice in the Description page of Project Settings.


#include "SentryAnimInstance.h"

USentryAnimInstance::USentryAnimInstance()
{
	_aimYawDeg = 0.0f;
	_aimPitchDeg = 0.0f;
	_aimAlpha = 1.0f;
	_extraSmoothingSpeed = 0.0f;
}

// -------------------------------------------------------
void USentryAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	// 초기화가 필요하면 여기서 처리
}

// -------------------------------------------------------
void USentryAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// 센트리에서 이미 보간했다면 아무 것도 하지 않음
	// 추가로 부드럽게 하고 싶다면 _extraSmoothingSpeed > 0.0f 로 두고 아래 보간 활성화
	if (_extraSmoothingSpeed > 0.0f)
	{
		// 예: 외부에서 목표 각도를 다른 변수로 넘기고 싶다면 그에 맞춰 보간
		// 현재는 _aimYawDeg / _aimPitchDeg 자체를 외부에서 최종값으로 세팅받으므로 비활성
	}
}

// -------------------------------------------------------
void USentryAnimInstance::SetAimAngles(float yawDeg, float pitchDeg)
{
	// 센트리에서 이미 각도 제한/보간을 끝낸 최종값을 넣어줍니다
	_aimYawDeg = yawDeg;
	_aimPitchDeg = pitchDeg;
}

// -------------------------------------------------------
void USentryAnimInstance::SetAimAlpha(float alpha)
{
	_aimAlpha = FMath::Clamp(alpha, 0.0f, 1.0f);
}