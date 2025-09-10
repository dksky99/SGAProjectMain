// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryWheelWidget.h"

#include "Components/TextBlock.h"

FReply UInventoryWheelWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    FVector2D mousePos = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
    FVector2D center = InGeometry.GetLocalSize() * 0.5f;

    FVector2D dir = mousePos - center;
    float dis = dir.Size();

    if (dis < centerRadius)
    {
        _curIndex = -1; // Ãë¼Ò
    }
    else
    {
        float angleRad = FMath::Atan2(dir.Y, dir.X); // -PI ~ +PI
        float angleDeg = FMath::RadiansToDegrees(angleRad); // -180 ~ +180
        angleDeg = FMath::Fmod(angleDeg + 360.f, 360.f); // 0 ~ 360

        _curIndex = FMath::FloorToInt(angleDeg / 90.0f);
    }

    FString text = FString::Printf(TEXT(""));

    switch (_curIndex)
    {
    case 3:
        text = FString::Printf(TEXT("Drop Gun"));
        break;
    case 2:
        text = FString::Printf(TEXT("Drop Item"));
        break;
    case 1:
        text = FString::Printf(TEXT("Drop Backpack"));
        break;
    case 0:
        text = FString::Printf(TEXT("Drop Sample"));
        break;
    default:
        text = FString::Printf(TEXT(""));
    }

	_indexText->SetText(FText::FromString(text));

    return FReply::Handled();
}
