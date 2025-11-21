// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetGlobeWidget.h"
#include "../Object/Map/PlanetOperationSite.h"
#include "../Object/Map/PlanetMissionIcon.h"
#include "../Data/OperationDataAsset.h"
#include "../Data/MissionDataAsset.h"
#include "../Data/ObjectiveDataAsset.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"

void UPlanetGlobeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	_bonusBoxes.Empty();
	_bonusBoxes.Add(_bonusBox1);
	_bonusBoxes.Add(_bonusBox2);
	_bonusBoxes.Add(_bonusBox3);

	_bonusTexts.Empty();
	_bonusTexts.Add(_bonusText1);
	_bonusTexts.Add(_bonusText2);
	_bonusTexts.Add(_bonusText3);

	for (auto& box : _bonusBoxes)
	{
		box->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPlanetGlobeWidget::EnterOperationMode()
{
	ShowMission(false);
	_widgetSwitcher->SetActiveWidgetIndex(0);
}

void UPlanetGlobeWidget::EnterMissionMode()
{
	ShowOperation(false);
	_widgetSwitcher->SetActiveWidgetIndex(1);
}

void UPlanetGlobeWidget::ShowOperation(bool visibility, APlanetOperationSite* site)
{
	if (!visibility)
	{
		if (_operationBox)
			_operationBox->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	if (!site) return;
	if (auto operation = site->GetOperationData())
	{
		_operationNameText->SetText(operation->GetOperationName());
		_operationDescText->SetText(operation->GetOperationDesc());

		for (int32 i = 0; i < operation->GetMissions().Num(); i++)
		{
			if (i >= _bonusBoxes.Num()) break;
			if (operation->GetRewardMedals().IsValidIndex(i))
			{
				_bonusBoxes[i]->SetVisibility(ESlateVisibility::Visible);
				_bonusTexts[i]->SetText(FText::AsNumber(operation->GetRewardMedals()[i]));
			}
		}
	}

	_operationBox->SetVisibility(ESlateVisibility::Visible);
}

void UPlanetGlobeWidget::ShowMission(bool visibility, APlanetMissionIcon* icon)
{
	if (!visibility)
	{
		if (_missionBox)
			_missionBox->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	if (!icon) return;
	if (auto mission = icon->GetMissionData())
	{
		_missionNameText->SetText(mission->GetMissionName());
		_missionDescText->SetText(mission->GetMissionDesc());
		_missionIcon->SetBrushFromTexture(mission->GetMissionIcon());
	}
	_missionBox->SetVisibility(ESlateVisibility::Visible);
}
