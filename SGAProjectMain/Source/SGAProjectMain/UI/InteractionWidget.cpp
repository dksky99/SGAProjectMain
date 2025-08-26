// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionWidget.h"
#include "Components/WidgetSwitcher.h"

void UInteractionWidget::ShowDefaultMark()
{
	_iconSwitcher->SetActiveWidgetIndex(0);
}

void UInteractionWidget::ShowKeyButtonMark()
{
	_iconSwitcher->SetActiveWidgetIndex(1);
}
