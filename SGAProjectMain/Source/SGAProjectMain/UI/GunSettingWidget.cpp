// Fill out your copyright notice in the Description page of Project Settings.

#include "GunSettingWidget.h"

#include "../Gun/GunBase.h"
#include "../Data/GunSettingSlotDataAsset.h"
#include "GunSettingSlotWidget.h"

#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"

void UGunSettingWidget::InitializeWidget(AGunBase* gun)
{
    _fireModePanel->ClearChildren();
    _scopeModePanel->ClearChildren();
    _lightModePanel->ClearChildren();

    UpdateGunInfo(gun->GetGunData(), gun->GetCurAmmo());
    SetSlots(gun->GetGunData());

    UpdateFireModePanel(gun->GetCurFireMode());
    UpdateLightModePanel(gun->GetCurLightMode());
    UpdateScopeModePanel(gun->GetCurScopeMode());
}

void UGunSettingWidget::UpdateGunInfo(const FGunData& gunData, int32 curAmmo)
{
	FString gunName = gunData._name.ToString();
	FString text = FString::Printf(TEXT("%s"), *gunName);
	_gunNameText->SetText(FText::FromString(text));

	text = FString::Printf(TEXT("%d / %d"), curAmmo, gunData._maxAmmo);
	_curAmmoText->SetText(FText::FromString(text));
}

void UGunSettingWidget::SetSlots(const FGunData& gunData)
{
    // 1. FireMode (오른쪽)
    for (EFireMode fireMode : gunData._fireModes)
    {
        if (fireEnum)
        {
            FName key = FName(fireEnum->GetNameStringByValue((int64)fireMode));
            UGunSettingSlotDataAsset* asset = FindSlotAsset(key);
            if (asset)
            {
                AddSlotToPanel(_fireModePanel, asset, false);
            }
        }
    }

    // 2. Range (위쪽)
    for (int32 scope : gunData._scopeModes)
    {
        FName key = FName(*FString::FromInt(scope));
        UGunSettingSlotDataAsset* asset = FindSlotAsset(key);
        if (asset)
        {
            AddSlotToPanel(_scopeModePanel, asset, true);
        }
    }

    // 3. Flashlight (아래쪽)
    for (ETacticalLightMode lightMode : gunData._lightModes)
    {
        if (lightEnum)
        {
            FName key = FName(lightEnum->GetNameStringByValue((int64)lightMode));
            UGunSettingSlotDataAsset* asset = FindSlotAsset(key);
            if (asset)
            {
                AddSlotToPanel(_lightModePanel, asset, false);
            }
        }
    }
}

void UGunSettingWidget::AddSlotToPanel(UPanelWidget* panel, UGunSettingSlotDataAsset* asset, bool isInsertReversed)
{
    UGunSettingSlotWidget* slot = CreateWidget<UGunSettingSlotWidget>(this, _slotWidgetClass);
    slot->InitializeSlot(asset);

    if (UVerticalBox* vertical = Cast<UVerticalBox>(panel))
    {
        // scopeMode
        if (isInsertReversed)
        {
            vertical->InsertChildAt(0, slot);
        }

        // lightMode
        else
        {
            vertical->AddChild(slot);
        }
    }

    // fireMode
    else if (UHorizontalBox* horizontal = Cast<UHorizontalBox>(panel))
    {
        horizontal->AddChild(slot);
    }
}

void UGunSettingWidget::UpdateFireModePanel(EFireMode fireMode)
{
    if (fireEnum)
    {
        FName key = FName(fireEnum->GetNameStringByValue((int64)fireMode));
        HighlightSlot(_fireModePanel, key);
    }
}

void UGunSettingWidget::UpdateLightModePanel(ETacticalLightMode lightMode)
{
    if (lightEnum)
    {
        FName key = FName(lightEnum->GetNameStringByValue((int64)lightMode));
        HighlightSlot(_lightModePanel, key);
    }
}

void UGunSettingWidget::UpdateScopeModePanel(int32 scopeMode)
{
    FName key = FName(*FString::FromInt(scopeMode));
    HighlightSlot(_scopeModePanel, key);
}

void UGunSettingWidget::HighlightSlot(UPanelWidget* panel, FName key)
{
    if (!IsValid(panel))
        return;

    int32 count = panel->GetChildrenCount();
    for (int32 i = 0; i < count; ++i)
    {
        UGunSettingSlotWidget* slot = Cast<UGunSettingSlotWidget>(panel->GetChildAt(i));
        if (!slot) continue;

        bool isCurMode = (slot->GetOptionKey() == key);
        slot->SetHighlight(isCurMode);
    }
}



UGunSettingSlotDataAsset* UGunSettingWidget::FindSlotAsset(FName key)
{
    for (auto* asset : _gunSlotDataAssets)
    {
        if (asset->_optionKey == key)
        {
            return asset;
        }
    }

    return nullptr;
}
