// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionResultWidget.h"
#include "MissionResultSlotWidget.h"
#include "../CGameInstance.h"
#include "../Game/PreDeployment/PreDeploymentState.h"
#include "../Data/MissionDataAsset.h"
#include "../Data/OperationDataAsset.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"

void UMissionResultWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	_missionIcons = { _missionIcon1, _missionIcon2, _missionIcon3 };
	_missionChecks = { _missionCheck1, _missionCheck2, _missionCheck3 };
	_bonusBoxes = { _bonusBox1, _bonusBox2, _bonusBox3 };
	_bonusTexts = { _bonusText1, _bonusText2, _bonusText3 };
}

void UMissionResultWidget::InitializeWidget(const FMissionResult& missionResult)
{
	_missionResult = missionResult;

	_rewardPanel->SetVisibility(ESlateVisibility::Visible);
	_xpBox->SetVisibility(ESlateVisibility::Visible);
	_requisitionBox->SetVisibility(ESlateVisibility::Visible);

	_operationPanel->SetVisibility(ESlateVisibility::Collapsed);
	_payoutPanel->SetVisibility(ESlateVisibility::Collapsed);

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
		}, 3.f, false);
}

void UMissionResultWidget::ShowRewardsSequentially()
{
	if (_rewardSlots.Num() == 0 || _curRewardIndex >= _rewardSlots.Num())
	{
		GetWorld()->GetTimerManager().ClearTimer(_rewardTimerHandle);
		FTimerHandle timerHandle;
		GetWorld()->GetTimerManager().SetTimer(timerHandle, [this]()
			{
				_curRewardIndex = 0;
				GetWorld()->GetTimerManager().SetTimer(_rewardTimerHandle, this, &UMissionResultWidget::HideRewardsSequentially, 0.5f, true);
			}, 2.f, false);
		return;
	}

	_rewardSlots[_curRewardIndex]->ShowRewardBox();
	_curRewardIndex++;
}

void UMissionResultWidget::HideRewardsSequentially()
{
	if (_rewardSlots.Num() == 0 || _curRewardIndex >= _rewardSlots.Num())
	{
		GetWorld()->GetTimerManager().ClearTimer(_rewardTimerHandle);
		ShowTotalRewards();
		return;
	}

	_rewardSlots[_curRewardIndex]->HideRewardBox();
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
			ShowOperationStatus();
		}, 3.f, false);
}

void UMissionResultWidget::ShowOperationStatus()
{
	_rewardPanel->SetVisibility(ESlateVisibility::Collapsed);
	_xpBox->SetVisibility(ESlateVisibility::Collapsed);
	_requisitionBox->SetVisibility(ESlateVisibility::Collapsed);
	_operationPanel->SetVisibility(ESlateVisibility::Visible);

	for (int32 i = 0; i < _missionIcons.Num(); ++i)
	{
		_missionIcons[i]->SetVisibility(ESlateVisibility::Collapsed);

		if (_missionChecks.IsValidIndex(i))
			_missionChecks[i]->SetVisibility(ESlateVisibility::Collapsed);

		if (_bonusBoxes.IsValidIndex(i))
		{
			_bonusBoxes[i]->SetVisibility(ESlateVisibility::Collapsed);
			_bonusBoxes[i]->SetRenderOpacity(0.5f);
		}
	}

	if (auto operation = _missionResult._operation)
	{
		auto GI = Cast<UCGameInstance>(GetGameInstance());
		if (!GI) return;

		const auto& missions = operation->GetMissions();
		const auto& rewardMedals = operation->GetRewardMedals();
		const TMap<UMissionDataAsset*, EMissionState>& missionStates = GI->GetPreDeployState()->GetMissionStates();

		for (int32 i = 0; i < missions.Num(); i++)
		{
			if (i >= _missionIcons.Num()) break;

			auto missionData = missions[i];
			if (!missionData) continue;

			// 미션 아이콘 설정
			_missionIcons[i]->SetBrushFromTexture(missionData->GetMissionIcon());
			_missionIcons[i]->SetVisibility(ESlateVisibility::Visible);
			if (const EMissionState* state = missionStates.Find(missionData))
			{
				if (*state == EMissionState::Cleared)
				{
					_missionChecks[i]->SetVisibility(ESlateVisibility::Hidden);	// 미리 공간 차지한 채로 숨김
					// 클리어된 미션은 2초 후에 체크 표시
					FTimerHandle checkTimerHandle;
					GetWorld()->GetTimerManager().SetTimer(checkTimerHandle, [this, i]()
						{
							_missionChecks[i]->SetVisibility(ESlateVisibility::Visible);
						}, 2.f, false);
				}
			}

			if (_bonusBoxes.IsValidIndex(i))
			{
				_bonusBoxes[i]->SetVisibility(ESlateVisibility::Visible);
				if (_bonusTexts.IsValidIndex(i) && rewardMedals.IsValidIndex(i))
					_bonusTexts[i]->SetText(FText::AsNumber(rewardMedals[i]));
			}
		}
	}

	FTimerHandle bonusTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(bonusTimerHandle, [this]()
		{
			for (int32 i = 0; i < _missionResult._clearedMissionNum; i++)
			{
				if (i >= _bonusTexts.Num()) break;
				_bonusTexts[i]->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
				_bonusBoxes[i]->SetRenderOpacity(1.f);
			}
		}, 3.f, false);

	FTimerHandle timerHandle;
	GetWorld()->GetTimerManager().SetTimer(timerHandle, [this]()
		{
			ShowPayoutSummary();
		}, 7.f, false);
}

void UMissionResultWidget::ShowPayoutSummary()
{
	_operationPanel->SetVisibility(ESlateVisibility::Collapsed);
	_payoutPanel->SetVisibility(ESlateVisibility::Visible);
	
	const FPlayerCurrency& totalReward = _missionResult._totalReward;

	_commonSampleText->SetText(FText::AsNumber(totalReward.GetSampleCount(ESampleType::Common)));
	_rareSampleText->SetText(FText::AsNumber(totalReward.GetSampleCount(ESampleType::Rare)));
	_superSampleText->SetText(FText::AsNumber(totalReward.GetSampleCount(ESampleType::Super)));
	_finalXpText->SetText(FText::AsNumber(totalReward._experience));
	_medalText->SetText(FText::AsNumber(totalReward._medals));
	_finalRequisitionText->SetText(FText::AsNumber(totalReward._requisitionSlips));

	FTimerHandle timerHandle;
	GetWorld()->GetTimerManager().SetTimer(timerHandle, [this]()
		{
			_rewardFlowFinishedEvent.ExecuteIfBound();
		}, 5.f, false);
}
