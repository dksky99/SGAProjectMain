// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionResultWidget.h"
#include "MissionResultSlotWidget.h"
#include "../MainGameMode.h"
#include "../Data/MissionDataAsset.h"
#include "Components/TextBlock.h"

void UMissionResultWidget::InitializeWidget(const FMissionResult& missionResult)
{
	_mainObjSlot->SetVisibility(ESlateVisibility::Collapsed);
	_optionalObjSlot->SetVisibility(ESlateVisibility::Collapsed);
	_extractedSlot->SetVisibility(ESlateVisibility::Collapsed);
	_timeSlot->SetVisibility(ESlateVisibility::Collapsed);

	int32 rewardSlotCount = 0;
	_rewardSlots.Empty();

	for (auto& reward : missionResult._missionRewards)
	{
		switch (reward._category)
		{
		case ERewardCategory::MainObjective:
			_mainObjSlot->InitializeSlot(missionResult, rewardSlotCount);
			break;

		case ERewardCategory::OptionalObjectives:
			if (missionResult._mission->HasOptionalObjectives())
				_optionalObjSlot->InitializeSlot(missionResult, rewardSlotCount);
			break;

		case ERewardCategory::HelldiversExtracted:
			_extractedSlot->InitializeSlot(missionResult, rewardSlotCount);
			break;

		case ERewardCategory::MissionTimeRemaining:
			_timeSlot->InitializeSlot(missionResult, rewardSlotCount);
			break;

		default:
			break;
		}

		rewardSlotCount++;
	}

	_totalXpText->SetText(FText::AsNumber(missionResult._totalReward._experience));
	_totalRequisitionText->SetText(FText::AsNumber(missionResult._totalReward._requisitionSlips));
}
