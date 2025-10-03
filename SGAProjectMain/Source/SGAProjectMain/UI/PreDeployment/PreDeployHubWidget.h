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
	void InitializeHubWidget(class UPreDeploymentState* state);

	FOnRequestLaunch _OnLaunchEvent;

protected:
	UFUNCTION()
	void SwitchToEquipPage();
	UFUNCTION()
	void OpenPrimaryEquipPanel(UPreDeployEntryBase* gunSlot);
	UFUNCTION()
	void OpenSecondaryEquipPanel(UPreDeployEntryBase* gunSlot);
	UFUNCTION()
	void ReturnToEquipPage();

	UFUNCTION()
	void SwitchToStratagemPage();
	void OpenStratagemPanel(bool isOpened);

	UFUNCTION()
	void HandleLaunchRequest();

protected:
	UPROPERTY(meta = (BindWidget))
	class UPreDeployPanelBase* _primaryEquipPanel;
	UPROPERTY(meta = (BindWidget))
	class UPreDeployPanelBase* _secondaryEquipPanel;
	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* _equipPanelSwitcher;
	
	UPROPERTY(meta = (BindWidget))
	class UPreDeployEntryBase* _primaryGunSlot;
	UPROPERTY(meta = (BindWidget))
	class UPreDeployEntryBase* _secondaryGunSlot;
	
	UPROPERTY(meta = (BindWidget))
	class UPreDeployStratagemPanel* _stratagemPanel;

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* _pageSwitcher; // 장비 <-> 스트라타젬
	UPROPERTY(meta = (BindWidget))
	class UButton* _equipBtn; // 스트라타젬에서 장비로
	UPROPERTY(meta = (BindWidget))
	class UButton* _stgBtn; // 장비에서 스트라타젬으로

	UPROPERTY(meta = (BindWidget))
	class UButton* _launchBtn;

	UPROPERTY(meta = (BindWidget))
	class UButton* _escBtn;	// 버튼으로 임시 구현

	// EIC 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input")
	class UInputAction* _navigateAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input")
	class UInputAction* _escAction;

	UPROPERTY()
	class UPreDeployPanelBase* _curPanel;

	UFUNCTION()
	void OnESC(const FInputActionValue& value);

	UFUNCTION()
	void OnNavigate(const FInputActionValue& value);
};
