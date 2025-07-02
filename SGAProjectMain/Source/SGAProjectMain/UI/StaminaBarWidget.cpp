// Fill out your copyright notice in the Description page of Project Settings.


#include "StaminaBarWidget.h"

#include "Components/ProgressBar.h"

void UStaminaBarWidget::SetStamina(float ratio, bool isRecover)
{
	_curStaminaPB->SetPercent(ratio);

	float blinkAlpha = FMath::Abs(FMath::Sin(GetWorld()->GetTimeSeconds() * 5.f));

	if (ratio == 0.f) // 스태미너를 모두 소모했을 때
	{
		//_curStaminaPB->SetFillColorAndOpacity(FLinearColor(1.f, 0.f, 0.f, blinkAlpha)); // 깜빡이는 빨강
	}
	else if (isRecover && ratio != 1.f) // 스태미너 회복 중일 때
	{
		_curStaminaPB->SetFillColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, blinkAlpha)); // 깜빡이는 하양
	}
	else
	{
		_curStaminaPB->SetFillColorAndOpacity(FLinearColor::White);
	}
}
