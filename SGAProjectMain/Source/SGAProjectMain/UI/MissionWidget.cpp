// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionWidget.h"

#include "MissionSlotWidget.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"

void UMissionWidget::AddMissionSlot(UTexture2D* texture, FString text)
{
	int32 num = _missionSlots->GetChildrenCount();
	if (num > 0)
	{
		auto slot = _missionSlots->GetChildAt(num - 1);
		if (auto missionSlot = Cast<UMissionSlotWidget>(slot))
		{
			missionSlot->DeactivateSlot();
		}

		ShowTempText();
	}
	
	auto slot = CreateWidget<UMissionSlotWidget>(this, _slotWidgetClass);
	if (slot)
	{
		slot->InitializeSlot(texture, text);
		_missionSlots->AddChild(slot);
	}
}

void UMissionWidget::ShowTempText()
{
	_tempText->SetVisibility(ESlateVisibility::Visible);

	FTimerHandle timerHandle;
	GetWorld()->GetTimerManager().SetTimer(timerHandle, [this]()
		{
			_tempText->SetVisibility(ESlateVisibility::Collapsed);
		}, 2.0f, false);
}
