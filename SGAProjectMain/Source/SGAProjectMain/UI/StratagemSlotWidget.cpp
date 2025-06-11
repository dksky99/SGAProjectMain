// Fill out your copyright notice in the Description page of Project Settings.


#include "StratagemSlotWidget.h"

#include "../Object/Stratagem/Stratagem.h"

#include "Components/Image.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"

void UStratagemSlotWidget::InitializeSlot(const AStratagem* stg)
{
	TArray<FKey> combo = stg->GetInputSequence();

	for (auto key : combo)
	{
        UImage* arrowImage = NewObject<UImage>(this);
        if (!arrowImage) continue;

        arrowImage->SetBrushFromTexture(_arrow);
        //arrowImage->SetColorAndOpacity(FLinearColor::Gray); // 초기엔 회색

        if (key == EKeys::W)
            arrowImage->SetRenderTransformAngle(-90.0f);
        else if (key == EKeys::S)
            arrowImage->SetRenderTransformAngle(90.0f);
        else if (key == EKeys::A)
            arrowImage->SetRenderTransformAngle(180.0f);
        //else if (key == EKeys::D) -> 회전 생략

        _stgCommands->AddChildToHorizontalBox(arrowImage);
	}
}

void UStratagemSlotWidget::ResetSlot()
{
    SetSlotOpacity(0.8f);
}

void UStratagemSlotWidget::UpdateSlot(int32 inputNum)
{
    _stgCommands->GetChildAt(inputNum - 1)->SetRenderOpacity(0.5f);
    _stgCommands->GetChildAt(inputNum)->SetRenderOpacity(1.f);

    _stgNameText->SetRenderOpacity(1.f);
    _stgIcon->SetRenderOpacity(1.f);
}

void UStratagemSlotWidget::DimSlot()
{
    SetSlotOpacity(0.5f);
}

void UStratagemSlotWidget::SetSlotOpacity(float opacity)
{
    auto commands = _stgCommands->GetAllChildren();

    for (auto command : commands)
    {
        command->SetRenderOpacity(opacity);
    }

    _stgNameText->SetRenderOpacity(opacity);
    _stgIcon->SetRenderOpacity(opacity);
}
