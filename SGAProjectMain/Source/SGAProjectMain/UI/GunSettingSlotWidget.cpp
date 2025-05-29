// Fill out your copyright notice in the Description page of Project Settings.


#include "GunSettingSlotWidget.h"

#include "../Data/GunSettingSlotDataAsset.h"

#include "Components/TextBlock.h"
#include "Components/Image.h"

void UGunSettingSlotWidget::InitializeSlot(UGunSettingSlotDataAsset* asset)
{
	_slotName->SetText(asset->_displayName);
	_slotImage->SetBrushFromTexture(asset->_icon);

	_optionKey = asset->_optionKey;
}

void UGunSettingSlotWidget::SetHighlight(bool isCurMode)
{
    float alpha = isCurMode ? 1.0f : 0.5f;

   if (_slotImage)
    {
        FLinearColor imgColor = _slotImage->ColorAndOpacity;
        imgColor.A = alpha;
        _slotImage->SetColorAndOpacity(imgColor);
    }

    if (_slotName)
    {
        FLinearColor txtColor = _slotName->ColorAndOpacity.GetSpecifiedColor();
        txtColor.A = alpha;
        _slotName->SetColorAndOpacity(FSlateColor(txtColor));
    }
}
