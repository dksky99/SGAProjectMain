// Fill out your copyright notice in the Description page of Project Settings.


#include "StratagemSlotWidget.h"

#include "../Object/Stratagem/Stratagem.h"

#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "StratagemWidget.h"

void UStratagemSlotWidget::InitializeSlot(const AStratagem* stg, UStratagemWidget* parentWidget)
{
	TArray<FKey> combo = stg->GetInputSequence();

    Super::InitializeSlot(combo);

    auto text = stg->GetStgName().ToString();
    _stgNameText->SetText(FText::FromString(text));

    _stgIcon->SetBrushFromTexture(stg->GetStgIcon());

    _parentWidget = parentWidget;
    ResetSlot();
}

void UStratagemSlotWidget::ResetSlot()
{
    Super::ResetSlot();

    _widgetSwitcher->SetActiveWidgetIndex(0);
    _slotState = EStgSlotWgtState::Normal;
}

void UStratagemSlotWidget::UpdateSlot(int32 comboNum)
{
    Super::UpdateSlot(comboNum);

    _stgNameText->SetRenderOpacity(1.f);
    _stgIcon->SetRenderOpacity(1.f);
}

void UStratagemSlotWidget::SetCooldown(float remainingTime)
{
    int32 minutes = FMath::FloorToInt(remainingTime / 60.0f);
    int32 seconds = FMath::FloorToInt(FMath::Fmod(remainingTime, 60.0f));
    
    auto text = FString::Printf(TEXT("%d:%02d"), minutes, seconds);
    _stgStateText->SetText(FText::FromString(text));
}

void UStratagemSlotWidget::SetSlotDeactivatingState()
{
    SetSlotOpacity(0.5f);
}

void UStratagemSlotWidget::SetSlotOperatingState()
{
    _widgetSwitcher->SetActiveWidgetIndex(1);

    FString text = FString::Printf(TEXT("Operating"));
    _stgStateText->SetText(FText::FromString(text));

    _slotState = EStgSlotWgtState::Operating;
    //_isShowingOperating = true;
    _isForcedShowing = true;
}

void UStratagemSlotWidget::SetSlotCooldownState(float remainingTime)
{
    _slotState = EStgSlotWgtState::Cooldown;

    _widgetSwitcher->SetActiveWidgetIndex(1);

    GetWorld()->GetTimerManager().ClearTimer(_startCooldownHandle);
    GetWorld()->GetTimerManager().ClearTimer(_preEndCooldownHandle);
    GetWorld()->GetTimerManager().ClearTimer(_endCooldownHandle);

    // 카운트다운 시작 3초 후 자동 닫힘
    GetWorld()->GetTimerManager().SetTimer(_startCooldownHandle, [this]()
        {
            //_isShowingOperating = false;
            _isForcedShowing = false;

            if (!_parentWidget->IsShowing())
                _parentWidget->OpenWidget(false);
        }, 3.f, false);

    if (remainingTime > 6.f)
    {
        GetWorld()->GetTimerManager().SetTimer(_preEndCooldownHandle, [this]()
            {
                _isForcedShowing = true;
                SetVisibility(ESlateVisibility::Visible);

                GetWorld()->GetTimerManager().SetTimer(_endCooldownHandle, [this]()
                    {
                        ResetSlot();
                        _isForcedShowing = false;

                        if (!_parentWidget->IsShowing())
                            _parentWidget->OpenWidget(false);
                    }, 6.f, false);
            }, remainingTime - 6.f, false);
    }

    //if (minutes == 0 && seconds == 5 && _cooldownTimerHandle.IsValid()) // 5초 남았을 때 다시 표시
    //{
    //    this->SetVisibility(ESlateVisibility::Visible);
    //    _isShowingCooldown = true;

    //    GetWorld()->GetTimerManager().SetTimer(_cooldownTimerHandle, [this]()
    //        {
    //            ResetSlot();
    //            _isShowingCooldown = false;

    //            if (!_parentWidget->IsShowing())
    //                _parentWidget->OpenWidget(false);
    //            _cooldownTimerHandle.Invalidate();
    //        }, 5.f, false);
    //}
}

void UStratagemSlotWidget::SetSlotOpacity(float opacity)
{
    Super::SetSlotOpacity(opacity);

    _stgNameText->SetRenderOpacity(opacity);
    _stgIcon->SetRenderOpacity(opacity);
}
