// Fill out your copyright notice in the Description page of Project Settings.


#include "CommandWidget.h"

#include "Components/WidgetSwitcher.h"

void UCommandWidget::InitializeSlot(const TArray<FKey>& combo)
{
    for (auto key : combo)
    {
        UImage* arrowImage = NewObject<UImage>(this);
        if (!arrowImage) continue;

        arrowImage->SetBrushFromTexture(_arrow);
        arrowImage->Brush.ImageSize = FVector2D(16.0f, 16.0f);
        arrowImage->SetDesiredSizeOverride(FVector2D(16.0f, 16.0f));
        //arrowImage->SetColorAndOpacity(FLinearColor::Gray); // 초기엔 회색

        if (key == EKeys::W)
            arrowImage->SetRenderTransformAngle(-90.0f);
        else if (key == EKeys::S)
            arrowImage->SetRenderTransformAngle(90.0f);
        else if (key == EKeys::A)
            arrowImage->SetRenderTransformAngle(180.0f);
        //else if (key == EKeys::D) -> 회전 생략

        _commandArrows->AddChildToHorizontalBox(arrowImage);
    }
}

void UCommandWidget::ResetSlot()
{
    if (_effectType == EArrowEffectType::Opacity)
        SetSlotOpacity(0.8f);
    else
    {
        auto commands = _commandArrows->GetAllChildren();
        for (auto command : commands)
        {
            if (auto image = Cast<UImage>(command))
            {
                image->SetColorAndOpacity(FLinearColor::White);
            }
		}
    }

    _widgetSwitcher->SetActiveWidgetIndex(0);
}

void UCommandWidget::UpdateSlot(int32 comboNum)
{
    if (!_commandArrows) return;

    if (_effectType == EArrowEffectType::Opacity)
    {
        _commandArrows->GetChildAt(comboNum - 1)->SetRenderOpacity(0.5f);
        _commandArrows->GetChildAt(comboNum)->SetRenderOpacity(1.f);
    }
    else
    {
        if (auto* image = Cast<UImage>(_commandArrows->GetChildAt(comboNum - 1)))
        {
            image->SetColorAndOpacity(FLinearColor::Yellow);
        }
    }
}

void UCommandWidget::OnCompleted()
{
    _widgetSwitcher->SetActiveWidgetIndex(1);
}

void UCommandWidget::SetSlotOpacity(float opacity)
{
    if (!_commandArrows) return;

    auto commands = _commandArrows->GetAllChildren();

    for (auto command : commands)
    {
        command->SetRenderOpacity(opacity);
    }
}
