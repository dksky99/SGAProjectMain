// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetGlobeWidget.h"
#include "Components/VerticalBox.h"

void UPlanetGlobeWidget::SetOperation(APlanetOperationSite* site, bool _visibility)
{
	if (!_visibility)
	{
		if (_operationBox)
			_operationBox->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	_operationBox->SetVisibility(ESlateVisibility::Visible);
}
