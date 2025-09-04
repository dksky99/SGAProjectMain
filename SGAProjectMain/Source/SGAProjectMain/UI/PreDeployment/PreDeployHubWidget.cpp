#include "PreDeployHubWidget.h"

#include "Components/Button.h"
#include "PreDeployPanelBase.h"


void UPreDeployHubWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (_launchBtn)
		_launchBtn->OnClicked.AddDynamic(this, &UPreDeployHubWidget::HandleLaunchRequest);
}

void UPreDeployHubWidget::InitializeWidget(UPreDeploymentState* state)
{
	// 패널들이랑 연결해주기
	_primary->InitializePanel(state);
	_secondary->InitializePanel(state);
	_support->InitializePanel(state);
}

void UPreDeployHubWidget::HandleLaunchRequest()
{
	if (_OnLaunchEvent.IsBound())
		_OnLaunchEvent.Broadcast();
}

