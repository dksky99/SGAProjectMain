// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionResultSlotWidget.h"
#include "../MainGameMode.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"

void UMissionResultSlotWidget::InitializeSlot(const FMissionResult& missionResult, int32 index)
{
	if (!missionResult._missionRewards.IsValidIndex(index))
		return;

	// 초기화
	//_rewardBox->SetVisibility(ESlateVisibility::Collapsed);
	_iconBox->SetVisibility(ESlateVisibility::Collapsed);
	_timeBox->SetVisibility(ESlateVisibility::Collapsed);

	this->SetVisibility(ESlateVisibility::Visible);
	
	// 보상 세팅
	auto reward = missionResult._missionRewards[index];

	switch (reward._category)
	{
	case ERewardCategory::MainObjective:
		_titleText->SetText(FText::FromString("Main Objectives"));
		_iconBox->SetVisibility(ESlateVisibility::Visible);
		break;

	case ERewardCategory::OptionalObjectives:
		_titleText->SetText(FText::FromString("Optional Objectives"));
		_iconBox->SetVisibility(ESlateVisibility::Visible);
		break;

	case ERewardCategory::HelldiversExtracted:
		_titleText->SetText(FText::FromString("Helldivers Extracted"));
		_iconBox->SetVisibility(ESlateVisibility::Visible);
		break;

	case ERewardCategory::MissionTimeRemaining:
		_titleText->SetText(FText::FromString("Mission Time Remaining"));
		_timeBox->SetVisibility(ESlateVisibility::Visible);
		_percentText->SetText(FText::AsPercent(missionResult._remainingTimeRatio));
		break;
		
	default:
		break;
	}

	_xpText->SetText(FText::AsNumber(reward._experience));
	_requisitionText->SetText(FText::AsNumber(reward._requisitionSlips));
}