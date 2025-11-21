// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionResultSlotWidget.h"
#include "../MainGameMode.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "../Data/MissionDataAsset.h"
#include "../Data/ObjectiveDataAsset.h"

void UMissionResultSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// 초기 상태
	this->SetVisibility(ESlateVisibility::Collapsed);

	_iconArray.Empty();
	_iconArray.Add(_icon1);
	_iconArray.Add(_icon2);
	_iconArray.Add(_icon3);
	_iconArray.Add(_icon4);

	for (auto& icon : _iconArray)
	{
		icon->SetOpacity(0.5f);
		icon->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UMissionResultSlotWidget::InitializeSlot(const FMissionResult& missionResult, int32 index)
{
	if (!missionResult._missionRewards.IsValidIndex(index))
		return;

	if (!missionResult._mission)
		return;

	// 초기화
	for (UImage* Icon : _iconArray)
	{
		if (!Icon) continue;

		Icon->SetOpacity(0.5f);
		Icon->SetVisibility(ESlateVisibility::Collapsed);
	}

	_rewardBox->SetVisibility(ESlateVisibility::Collapsed);
	_iconBox->SetVisibility(ESlateVisibility::Collapsed);
	_timeBox->SetVisibility(ESlateVisibility::Collapsed);

	SetVisibility(ESlateVisibility::Visible);
	
	// 보상 세팅
	auto reward = missionResult._missionRewards[index];

	switch (reward._category)
	{
	case ERewardCategory::MainObjective:
		_titleText->SetText(FText::FromString("Main Objectives"));
		_iconBox->SetVisibility(ESlateVisibility::Visible);

		if (auto mainObj = missionResult._mission->GetMainObjective())
		{
			_icon1->SetBrushFromTexture(mainObj->GetObjectiveIcon());
			_icon1->SetVisibility(ESlateVisibility::Visible);
			if (missionResult._isMainObjectiveCleared)
				_icon1->SetOpacity(1.f);
		}
		break;

	case ERewardCategory::OptionalObjectives:
	{
		_titleText->SetText(FText::FromString("Optional Objectives"));
		_iconBox->SetVisibility(ESlateVisibility::Visible);

		auto optionalObjectives = missionResult._mission->GetOptionalObjectives();
		for (int i = 0; i < FMath::Min(optionalObjectives.Num(), _iconArray.Num()); i++)
		{
			auto optionalObj = optionalObjectives[i];
			if (!optionalObj)
				continue;
			_iconArray[i]->SetBrushFromTexture(optionalObj->GetObjectiveIcon());
			_iconArray[i]->SetVisibility(ESlateVisibility::Visible);
			if (missionResult._completedOptionalObjectives.Contains(optionalObj->GetObjectiveID()))
				_iconArray[i]->SetOpacity(1.f);
		}
	}
		break;

	case ERewardCategory::HelldiversExtracted:
		_titleText->SetText(FText::FromString("Helldivers Extracted"));
		_iconBox->SetVisibility(ESlateVisibility::Visible);
		for (int32 i = 0; i < FMath::Min(missionResult._extractedHelldiversNum, _iconArray.Num()); i++)
		{
			_iconArray[i]->SetVisibility(ESlateVisibility::Visible);
			_iconArray[i]->SetOpacity(1.f);
		}
		_iconArray[0]->SetVisibility(ESlateVisibility::Visible); // 최소 1개는 아이콘 표시
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

void UMissionResultSlotWidget::ShowRewardBox()
{
	_rewardBox->SetVisibility(ESlateVisibility::Visible);
	// 나머지 50% 투명 처리
	_timeBox->SetRenderOpacity(0.5f);
	_iconBox->SetRenderOpacity(0.5f);
}

void UMissionResultSlotWidget::HideRewardBox()
{
	_rewardBox->SetVisibility(ESlateVisibility::Collapsed);
	// 나머지 원래대로
	_timeBox->SetRenderOpacity(1.f);
	_iconBox->SetRenderOpacity(1.f);
}
