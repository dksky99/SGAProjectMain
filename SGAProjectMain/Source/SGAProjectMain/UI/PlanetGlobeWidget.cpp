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
		if (auto mainObj = mission->GetMainObjective())
			_missionIcon->SetBrushFromTexture(mainObj->GetObjectiveIcon());
	}
	_missionBox->SetVisibility(ESlateVisibility::Visible);
}
