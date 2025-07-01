// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniMapWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void UMiniMapWidget::ResetMap()
{
    _cursorPosText->SetVisibility(ESlateVisibility::Hidden);
}

void UMiniMapWidget::SetCursorText(FVector sceneCapturerToCursor, FVector playerToCursor, float halfWidth)
{
    _cursorPosText->SetVisibility(ESlateVisibility::Visible);

	// UV 얻기
	// U, V의 범위는 0 ~ 1
	float U = ((sceneCapturerToCursor.Y + halfWidth) / (2 * halfWidth));
	float V = 1.0f - (sceneCapturerToCursor.X + halfWidth) / (2 * halfWidth);
	FVector2D UV = { U, V };

	// 거리 얻기
	float dist = playerToCursor.Size2D();

	// 방향 얻기
	float rad = FMath::Atan2(playerToCursor.Y, playerToCursor.X); // XY 평면 기준 각도
	float deg = FMath::RadiansToDegrees(rad);
	if (deg < 0) deg += 360.f;

	// 방향 문자열
	FString dir;
	if (deg < 22.5f || deg >= 337.5f) dir = TEXT("East");
	else if (deg < 67.5f) dir = TEXT("SouthEast");
	else if (deg < 112.5f) dir = TEXT("South");
	else if (deg < 157.5f) dir = TEXT("SouthWest");
	else if (deg < 202.5f) dir = TEXT("West");
	else if (deg < 247.5f) dir = TEXT("NorthWest");
	else if (deg < 292.5f) dir = TEXT("North");
	else dir = TEXT("NorthEast");

    // 미니맵 이미지의 위젯에서의 위치와 크기
    FVector2D mapSize = _mapImage->GetCachedGeometry().GetLocalSize(); // 크기
    FVector2D canvasSize = _canvasPanel->GetCachedGeometry().GetLocalSize(); // _rootCanvas는 UCanvasPanel*
    FVector2D mapTopLeft = canvasSize - mapSize - FVector2D(23.f, 37.f);

    // 텍스트 위치 오프셋
    FVector2D posOffset = UV * mapSize; // 맵에서 커서 위치
    FVector2D finalPos = mapTopLeft + posOffset + FVector2D(10.f, 10.f); // 맵의 좌상단으로부터 상대적인 위치만큼 + 커서보다 우하단

    // 위치 적용
    if (UCanvasPanelSlot* slot = Cast<UCanvasPanelSlot>(_cursorPosText->Slot))
    {
        slot->SetPosition(finalPos);
    }

    FString text = FString::Printf(TEXT("%s(%.0f°)\n%.0fm"), *dir, deg, dist / 100.f);
    _cursorPosText->SetText(FText::FromString(text));
}

void UMiniMapWidget::SetPingImage(FVector sceneCapturerToPing, float halfWidth)
{
	// 미니맵 이미지의 위젯에서의 위치와 크기
	FVector2D mapSize = _mapImage->GetCachedGeometry().GetLocalSize(); // 크기
	FVector2D canvasSize = _canvasPanel->GetCachedGeometry().GetLocalSize(); // _rootCanvas는 UCanvasPanel*
	FVector2D finalPos;

	float dist = sceneCapturerToPing.Size2D();

	if (dist <= halfWidth) // 미니맵 안에 핑 존재 -> 해당 위치에 표시
	{
		float U = ((sceneCapturerToPing.Y + halfWidth) / (2 * halfWidth));
		float V = 1.0f - (sceneCapturerToPing.X + halfWidth) / (2 * halfWidth);

		FVector2D mapTopLeft = canvasSize - mapSize - FVector2D(23.f, 37.f);
		finalPos = mapTopLeft + FVector2D(U, V) * mapSize;
	}
	else // 원 밖 → 방향 표시
	{
		float rad = FMath::Atan2(sceneCapturerToPing.Y, sceneCapturerToPing.X);
		float deg = FMath::RadiansToDegrees(rad);

		FVector2D mapCenter = canvasSize - mapSize * 0.5f - FVector2D(23.f, 37.f);
		float X = mapCenter.X + mapSize.X * 0.45f * FMath::Sin(rad);
		float Y = mapCenter.Y - mapSize.Y * 0.45f * FMath::Cos(rad);

		finalPos = FVector2D(X, Y);
	}

	// 위치 적용
	if (UCanvasPanelSlot* slot = Cast<UCanvasPanelSlot>(_pingImage->Slot))
	{
		slot->SetPosition(finalPos); // 사진 크기 절반만큼 이동
	}
}

void UMiniMapWidget::ShowPingImage(bool needToShow)
{
	if (needToShow)
		_pingImage->SetVisibility(ESlateVisibility::Visible);
	else
		_pingImage->SetVisibility(ESlateVisibility::Hidden);
}
