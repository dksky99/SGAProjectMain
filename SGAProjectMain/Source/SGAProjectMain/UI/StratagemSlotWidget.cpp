// Fill out your copyright notice in the Description page of Project Settings.


#include "StratagemSlotWidget.h"

#include "../Object/Stratagem/Stratagem.h"

#include "Components/Image.h"
#include "Components/HorizontalBox.h"

void UStratagemSlotWidget::InitializeStgSlot(const AStratagem* stg)
{
	TArray<FKey> combo = stg->GetInputSequence();

	for (auto key : combo)
	{
        UImage* arrowImage = NewObject<UImage>(this);
        if (!arrowImage) continue;

        UTexture2D* arrowTexture;

        if (key == EKeys::W)
            arrowTexture = _upArrow;
        else if (key == EKeys::S)
            arrowTexture = _downArrow;
        else if (key == EKeys::A)
            arrowTexture = _leftArrow;
        else if (key == EKeys::D)
            arrowTexture = _rightArrow;
        else
            return;

        arrowImage->SetBrushFromTexture(arrowTexture);
        arrowImage->SetColorAndOpacity(FLinearColor::Gray); // 초기엔 회색

        _stgCommands->AddChildToHorizontalBox(arrowImage);
	}
}

void UStratagemSlotWidget::UpdateStgSlot(const TArray<FKey>& inputBuffer)
{
    for (int32 i = 0; i < inputBuffer.Num(); i++)
    {
        Cast<UImage>(_stgCommands->GetChildAt(i))->SetColorAndOpacity(FLinearColor::White);
    }
}

void UStratagemSlotWidget::ResetSlot()
{
    auto commands = _stgCommands->GetAllChildren();

    for (auto command : commands)
    {
        Cast<UImage>(command)->SetColorAndOpacity(FLinearColor::Gray);
    }
}
