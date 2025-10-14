// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetGlobeWidget.h"
#include "Components/Border.h"
#include "Components/WidgetSwitcher.h"

void UPlanetGlobeWidget::ShowOperation(bool visibility, APlanetOperationSite* site)
{
	_widgetSwitcher->SetActiveWidgetIndex(0);

	if (!visibility)
	{
		if (_operationBox)
			_operationBox->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	if (!site) return;

	_operationBox->SetVisibility(ESlateVisibility::Visible);
}

void UPlanetGlobeWidget::ShowObjection(bool visibility, APlanetObjectiveIcon* icon)
{
	_widgetSwitcher->SetActiveWidgetIndex(1);

	if (!visibility)
	{
		if (_objectiveBox)
			_objectiveBox->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	if (!icon) return;
	_objectiveBox->SetVisibility(ESlateVisibility::Visible);
}
