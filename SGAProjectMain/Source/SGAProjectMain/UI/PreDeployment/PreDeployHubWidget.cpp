#include "PreDeployHubWidget.h"

#include "Components/Button.h"


void UPreDeployHubWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (_launchBtn)
		_launchBtn->OnClicked.AddDynamic(this, &UPreDeployHubWidget::HandleLaunchRequest);
}

void UPreDeployHubWidget::InitializeWidget(UPreDeploymentState* state)
{
	// 패널들이랑 연결해주기
}

void UPreDeployHubWidget::HandleLaunchRequest()
{
	if (_OnLaunchEvent.IsBound())
		_OnLaunchEvent.Broadcast();
}

