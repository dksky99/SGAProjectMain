// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionSlotWidget.h"

#include "Components/TextBlock.h"
#include "Components/Image.h"

void UMissionSlotWidget::InitializeSlot(UTexture2D* texture, FString name)
{
	_missionNameText->SetText(FText::FromString(name));
	_missionIcon->SetBrushFromTexture(texture);
}

void UMissionSlotWidget::DeactivateSlot()
{
	_missionIcon->SetRenderOpacity(0.5f);
	_missionNameText->SetRenderOpacity(0.5f);
}
