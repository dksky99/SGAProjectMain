// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PreDeployHubWidget.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRequestLaunch);

UCLASS()
class SGAPROJECTMAIN_API UPreDeployHubWidget : public UUserWidget
{
	GENERATED_BODY()
	
	virtual void NativeOnInitialized() override;

public:
	void InitializeWidget(class UPreDeploymentState* state);

	FOnRequestLaunch _OnLaunchEvent;

protected:
	UFUNCTION()
	void HandleLaunchRequest();

	UPROPERTY(meta = (BindWidget))
	class UButton* _launchBtn;

	UPROPERTY(meta = (BindWidget))
	class UPreDeployPanelBase* _primary;
	UPROPERTY(meta = (BindWidget))
	class UPreDeployPanelBase* _secondary;
	UPROPERTY(meta = (BindWidget))
	class UPreDeployPanelBase* _support;

	UPROPERTY(meta = (BindWidget))
	class UPreDeployStratagemPanel* _stratagem;
};
