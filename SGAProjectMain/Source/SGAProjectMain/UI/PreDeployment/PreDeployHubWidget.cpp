#include "PreDeployHubWidget.h"

#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "PreDeployPanelBase.h"
#include "PreDeployStratagemPanel.h"


void UPreDeployHubWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (_launchBtn)
		_launchBtn->OnClicked.AddDynamic(this, &UPreDeployHubWidget::HandleLaunchRequest);

	_equipBtn->OnClicked.AddDynamic(this, &UPreDeployHubWidget::SwitchToEquipPage);
	_stgBtn->OnClicked.AddDynamic(this, &UPreDeployHubWidget::SwitchToStratagemPage);

	_primaryGunBtn->OnClicked.AddDynamic(this, &UPreDeployHubWidget::OpenPrimaryEquipPanel);
	_secondaryGunBtn->OnClicked.AddDynamic(this, &UPreDeployHubWidget::OpenSecondaryPanel);

	_escBtn->OnClicked.AddDynamic(this, &UPreDeployHubWidget::OnESCPressed);
}

void UPreDeployHubWidget::InitializeHubWidget(UPreDeploymentState* state)
{
	// 패널들이랑 연결해주기
	_primaryEquipPanel->InitializePanel(state);
	_secondaryEquipPanel->InitializePanel(state);
	_stratagemPanel->InitializePanel(state);
	_stratagemPanel->_panelOpenedEvent.AddUObject(this, &UPreDeployHubWidget::OpenStratagemPanel);
}

void UPreDeployHubWidget::OnESCPressed()
{
	if (_hubSwitcher->GetActiveWidgetIndex() != 0) // 장비 패널 열려있으면
	{
		ReturnToEquipPage(); // 장비 페이지로 돌아가기
	}
	else if (_pageSwitcher->GetActiveWidgetIndex() == 1) // 스트라타젬 페이지라면
	{
		if (_stratagemPanel->IsPanelOpen()) 
			_stratagemPanel->ClosePanel(); // 패널이 열려있으면 닫기
	}
}

void UPreDeployHubWidget::HandleLaunchRequest()
{
	if (_OnLaunchEvent.IsBound())
		_OnLaunchEvent.Broadcast();
}

void UPreDeployHubWidget::SwitchToEquipPage()
{
	_pageSwitcher->SetActiveWidgetIndex(0);
}

void UPreDeployHubWidget::SwitchToStratagemPage()
{
	_pageSwitcher->SetActiveWidgetIndex(1);
}

void UPreDeployHubWidget::OpenStratagemPanel(bool isOpened)
{
	if (isOpened)
	{
		_equipBtn->SetVisibility(ESlateVisibility::Hidden);
		_launchBtn->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		_equipBtn->SetVisibility(ESlateVisibility::Visible);
		_launchBtn->SetVisibility(ESlateVisibility::Visible);
	}
}

void UPreDeployHubWidget::OpenPrimaryEquipPanel()
{
	_hubSwitcher->SetActiveWidgetIndex(1);
}

void UPreDeployHubWidget::OpenSecondaryPanel()
{
	_hubSwitcher->SetActiveWidgetIndex(2);
}

void UPreDeployHubWidget::ReturnToEquipPage()
{
	_hubSwitcher->SetActiveWidgetIndex(0);
	_pageSwitcher->SetActiveWidgetIndex(0);
}
