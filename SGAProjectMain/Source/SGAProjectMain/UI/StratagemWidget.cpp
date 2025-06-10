// Fill out your copyright notice in the Description page of Project Settings.


#include "StratagemWidget.h"

#include "StratagemSlotWidget.h"
#include "../StratagemComponent.h"
#include "../Object/Stratagem/Stratagem.h"

#include "Components/VerticalBox.h"

void UStratagemWidget::InitializeStgWidget(const TArray<FStratagemSlot>& stgSlots)
{
	for (auto stgSlot : stgSlots)
	{
		auto stgClass = stgSlot.StratagemClass;
		const AStratagem* stg = stgClass->GetDefaultObject<AStratagem>();

		UStratagemSlotWidget* slot = CreateWidget<UStratagemSlotWidget>(this, _slotWidgetClass);
		slot->InitializeStgSlot(stg);
		_stgSlots->AddChild(slot);
	}
}

void UStratagemWidget::UpdateStgWidget(int32 index, const TArray<FKey> inputBuffer)
{
	auto stgSlotWdg = Cast<UStratagemSlotWidget>(_stgSlots->GetChildAt(index));
	stgSlotWdg->UpdateStgSlot(inputBuffer);
}
