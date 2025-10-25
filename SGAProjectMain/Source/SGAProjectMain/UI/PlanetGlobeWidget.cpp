// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetGlobeWidget.h"
#include "Components/Border.h"
#include "Components/WidgetSwitcher.h"

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
	_missionBox->SetVisibility(ESlateVisibility::Visible);
}
