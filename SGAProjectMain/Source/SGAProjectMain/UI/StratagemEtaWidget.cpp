// Fill out your copyright notice in the Description page of Project Settings.

#include "StratagemEtaWidget.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

void UStratagemEtaWidget::SetEtaSeconds(int32 etaSec)
{
	if (!_etaText)
		return;

	const int32 clamped = FMath::Max(0, etaSec);
	const FText text = FText::FromString(FString::Printf(TEXT("%d s"), clamped));
	_etaText->SetText(text);
}

void UStratagemEtaWidget::SetWidgetScreenPosition(const FVector2D& screenPosition)
{
	if (!_etaBorder)
		return;

	// 위젯의 가운데를 기준으로
	SetAlignmentInViewport(FVector2D(0.5f, 0.5f));   

	// 최종적으로 이 위젯 중심이 가야 할 "뷰포트 좌표"를 그대로 사용
	// bRemoveDPIScale = true 로 두어 ProjectWorldLocationToScreen 결과를 바로 쓸 수 있게 한다
	SetPositionInViewport(screenPosition, true);
}

void UStratagemEtaWidget::SetIndicatorVisible(bool isVisible)
{
	SetVisibility(isVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}
