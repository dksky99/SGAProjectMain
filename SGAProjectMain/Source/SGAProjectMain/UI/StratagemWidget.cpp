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
		slot->InitializeSlot(stg);
		_stgSlots->AddChild(slot);
	}

	ResetWidget();
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
		stgSlotWdg->DimSlot();
	}
}

void UStratagemWidget::ResetWidget()
{
	auto stgSlots = _stgSlots->GetAllChildren();
	for (auto slot : stgSlots)
	{
		Cast<UStratagemSlotWidget>(slot)->ResetSlot();
	}
}
