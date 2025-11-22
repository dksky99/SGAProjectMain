// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionSlotWidget.h"

#include "Components/TextBlock.h"
#include "Components/Image.h"

void UMissionSlotWidget::InitializeSlot(UTexture2D* texture, FText name, FName ID)
{
	_missionID = ID;
	_missionNameText->SetText(name);
	_missionIcon->SetBrushFromTexture(texture);
}

void UMissionSlotWidget::DeactivateSlot()
{
	_missionID = NAME_None; // 필요 없으므로 초기화
	_missionIcon->SetRenderOpacity(0.5f);
	_missionNameText->SetRenderOpacity(0.5f);
}
