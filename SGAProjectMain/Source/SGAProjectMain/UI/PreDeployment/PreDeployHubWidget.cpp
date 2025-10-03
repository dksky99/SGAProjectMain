#include "PreDeployHubWidget.h"

#include "EnhancedInputComponent.h"
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

	UCGameInstance* GI = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
	auto state = GI->GetPreDeployState();
	_primaryGunSlot->InitializeEntry(state->GetPrimaryGunID());
	_secondaryGunSlot->InitializeEntry(state->GetSecondaryGunID());

	_primaryGunSlot->_onPickedEvent.AddUObject(this, &UPreDeployHubWidget::OpenPrimaryEquipPanel);
	_secondaryGunSlot->_onPickedEvent.AddUObject(this, &UPreDeployHubWidget::OpenSecondaryEquipPanel);

	//_escBtn->OnClicked.AddDynamic(this, &UPreDeployHubWidget::OnESC);

	if (auto* EIC = Cast<UEnhancedInputComponent>(GetOwningPlayer()->InputComponent))
	{
		EIC->BindAction(_navigateAction, ETriggerEvent::Started, this, &UPreDeployHubWidget::OnNavigate);
		EIC->BindAction(_escAction, ETriggerEvent::Started, this, &UPreDeployHubWidget::OnESC);
	}
}

void UPreDeployHubWidget::InitializeHubWidget(UPreDeploymentState* state)
{
	// 패널들이랑 연결해주기
	_primaryEquipPanel->InitializePanel(state);
	_primaryEquipPanel->_selectChangedEvent.AddUObject(_primaryGunSlot, &UPreDeployEntryBase::InitializeEntry);
	_secondaryEquipPanel->InitializePanel(state);
	_secondaryEquipPanel->_selectChangedEvent.AddUObject(_secondaryGunSlot, &UPreDeployEntryBase::InitializeEntry);
	_stratagemPanel->InitializePanel(state);
	_stratagemPanel->_panelOpenedEvent.AddUObject(this, &UPreDeployHubWidget::OpenStratagemPanel);
}

void UPreDeployHubWidget::SwitchToEquipPage()
{
	_pageSwitcher->SetActiveWidgetIndex(0);
	_curPanel = nullptr;
}

void UPreDeployHubWidget::OpenPrimaryEquipPanel(UPreDeployEntryBase* gunSlot)
{
	_equipPanelSwitcher->SetActiveWidgetIndex(1);
	gunSlot->SetSelected(true);
	_secondaryGunSlot->SetSelected(false);
	_curPanel = _primaryEquipPanel;
}

void UPreDeployHubWidget::OpenSecondaryEquipPanel(UPreDeployEntryBase* gunSlot)
{
	_equipPanelSwitcher->SetActiveWidgetIndex(2);
	gunSlot->SetSelected(true);
	_primaryGunSlot->SetSelected(false);
	_curPanel = _secondaryEquipPanel;
}

void UPreDeployHubWidget::ReturnToEquipPage()
{
	_equipPanelSwitcher->SetActiveWidgetIndex(0);
	_pageSwitcher->SetActiveWidgetIndex(0);
	_curPanel = nullptr;
}

void UPreDeployHubWidget::SwitchToStratagemPage()
{
	_pageSwitcher->SetActiveWidgetIndex(1);
	_curPanel = nullptr;
}

void UPreDeployHubWidget::OpenStratagemPanel(bool isOpened)
{
	if (isOpened)
	{
		_equipBtn->SetVisibility(ESlateVisibility::Hidden);
		_launchBtn->SetVisibility(ESlateVisibility::Hidden);
		_curPanel = nullptr;
	}
	else
	{
		_equipBtn->SetVisibility(ESlateVisibility::Visible);
		_launchBtn->SetVisibility(ESlateVisibility::Visible);
		_curPanel = _stratagemPanel;
	}
}

void UPreDeployHubWidget::HandleLaunchRequest()
{
	if (_OnLaunchEvent.IsBound())
		_OnLaunchEvent.Broadcast();
}

void UPreDeployHubWidget::OnESC(const FInputActionValue& value)
{
	if (_equipPanelSwitcher->GetActiveWidgetIndex() != 0) // 장비 패널 열려있으면
	{
		ReturnToEquipPage(); // 장비 페이지로 돌아가기
	}
	else if (_pageSwitcher->GetActiveWidgetIndex() == 1) // 스트라타젬 페이지라면
	{
		if (_stratagemPanel->IsPanelOpen()) 
			_stratagemPanel->ClosePanel(); // 패널이 열려있으면 닫기
	}
}

void UPreDeployHubWidget::OnNavigate(const FInputActionValue& value)
{
	if (!_curPanel) return;

	FVector2D dir = value.Get<FVector2D>();
	if (dir.X > 0) _curPanel->MoveRight();
	else if (dir.X < 0) _curPanel->MoveLeft();
	else if (dir.Y > 0) _curPanel->MoveUp();
	else if (dir.Y < 0) _curPanel->MoveDown();
}