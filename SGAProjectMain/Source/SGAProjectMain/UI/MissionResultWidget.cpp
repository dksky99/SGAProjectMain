// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionResultWidget.h"
#include "MissionResultSlotWidget.h"
#include "../Data/MissionDataAsset.h"
#include "Components/TextBlock.h"

void UMissionResultWidget::InitializeWidget(const FMissionResult& missionResult)
{
	_missionResult = missionResult;

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
			_rewardSlots.Add(_mainObjSlot);
			break;

		case ERewardCategory::OptionalObjectives:
			if (missionResult._mission->HasOptionalObjectives())
			{
				_optionalObjSlot->InitializeSlot(missionResult, rewardSlotCount);
				_rewardSlots.Add(_optionalObjSlot);
			}
			break;

		case ERewardCategory::HelldiversExtracted:
			_extractedSlot->InitializeSlot(missionResult, rewardSlotCount);
			_rewardSlots.Add(_extractedSlot);
			break;

		case ERewardCategory::MissionTimeRemaining:
			_timeSlot->InitializeSlot(missionResult, rewardSlotCount);
			_rewardSlots.Add(_timeSlot);
			break;

		default:
			break;
		}

		rewardSlotCount++;
	}

	FTimerHandle timerHandle;
	GetWorld()->GetTimerManager().SetTimer(timerHandle, [this]()
		{
			_curRewardIndex = 0;
			GetWorld()->GetTimerManager().SetTimer(_rewardTimerHandle, this, &UMissionResultWidget::ShowRewardsSequentially, 0.5f, true);
		}, 5.f, false);
}

void UMissionResultWidget::ShowRewardsSequentially()
{
	if (_rewardSlots.Num() == 0 || _curRewardIndex >= _rewardSlots.Num())
	{
		GetWorld()->GetTimerManager().ClearTimer(_rewardTimerHandle);
		ShowTotalRewards();
		return;
	}

	_rewardSlots[_curRewardIndex]->ShowRewardBox();
	_curRewardIndex++;
}

void UMissionResultWidget::ShowTotalRewards()
{
	for (auto& slot : _rewardSlots)
		slot->HideRewardBox();

	_totalXpText->SetText(FText::AsNumber(_missionResult._totalReward._experience));
	_totalRequisitionText->SetText(FText::AsNumber(_missionResult._totalReward._requisitionSlips));

	FTimerHandle timerHandle;
	GetWorld()->GetTimerManager().SetTimer(timerHandle, [this]()
		{
			_rewardFlowFinishedEvent.ExecuteIfBound();
		}, 5.f, false);
}
