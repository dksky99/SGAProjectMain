// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "../Object/Interactable.h"

void UInteractionWidget::InitializeWidget(FInteractableInfo info)
{
	_interactionTxtBlock->SetText(info._interactionText);
	
	switch (info._type)
	{
	case EInteractableIconType::None:
		break;
	case EInteractableIconType::Gun:
		break;
	case EInteractableIconType::Sample:
		break;
	case EInteractableIconType::SupplyBox:
		break;
	}

	_markSwitcher->SetActiveWidgetIndex(0);
}

void UInteractionWidget::ShowDefaultMark()
{
	_markSwitcher->SetActiveWidgetIndex(0);
	_interactionTxtBlock->SetVisibility(ESlateVisibility::Hidden);
}

void UInteractionWidget::ShowKeyButtonMark()
{
	_markSwitcher->SetActiveWidgetIndex(1);
	_interactionTxtBlock->SetVisibility(ESlateVisibility::Visible);
}
