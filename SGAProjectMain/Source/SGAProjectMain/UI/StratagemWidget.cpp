// Fill out your copyright notice in the Description page of Project Settings.


#include "StratagemWidget.h"

#include "StratagemSlotWidget.h"
#include "../StratagemComponent.h"
#include "../Object/Stratagem/Stratagem.h"

#include "Components/VerticalBox.h"

void UStratagemWidget::InitializeWidget(const TArray<FStratagemSlot>& stgSlots)
{
	for (const FStratagemSlot& stgSlot : stgSlots)
	{
		auto stgClass = stgSlot.StratagemClass;
		const AStratagem* stg = stgClass->GetDefaultObject<AStratagem>();

		UStratagemSlotWidget* slot = CreateWidget<UStratagemSlotWidget>(this, _slotWidgetClass);
		slot->InitializeSlot(stg, this);
		_stgSlots->AddChild(slot);
	}
}

void UStratagemWidget::UpdateWidget(int32 stgIndex, int32 inputNum, bool bPrefixMax)
{
	auto stgSlotWdg = Cast<UStratagemSlotWidget>(_stgSlots->GetChildAt(stgIndex));

	if (bPrefixMax)
	{
		stgSlotWdg->UpdateSlot(inputNum);
	}
	else
	{
		stgSlotWdg->SetSlotDeactivatingState();
	}
}

void UStratagemWidget::OpenWidget(bool visible)
{
	auto stgSlots = _stgSlots->GetAllChildren();
	for (auto slot : stgSlots)
	{
		if (visible)
		{
			_isShowing = true;
			slot->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			auto stgSlot = Cast<UStratagemSlotWidget>(slot);
			if (!stgSlot->IsForcedShowing())
			{
				if (stgSlot->GetSlotState() != EStgSlotWdgState::Cooldown)
					stgSlot->ResetSlot();
				stgSlot->SetVisibility(ESlateVisibility::Hidden);
			}
			_isShowing = false;
		}
	}
}

void UStratagemWidget::SetWidgetOperatingState(int32 index)
{
	auto stgSlots = _stgSlots->GetAllChildren();

	Cast<UStratagemSlotWidget>(stgSlots[index])->SetSlotOperatingState();
	
	OpenWidget(false);
}

void UStratagemWidget::SetWidgetCooldownState(int32 index, float remainingTime)
{
	auto slots = _stgSlots->GetAllChildren();
	auto stgSlot = Cast<UStratagemSlotWidget>(slots[index]);
		
	if (stgSlot->GetSlotState() == EStgSlotWdgState::Operating)
		stgSlot->SetSlotCooldownState(remainingTime);

	stgSlot->SetCooldown(remainingTime);
}
