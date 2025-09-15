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
	// 원래대로라면 function을 위쪽에 모아놓지만,
	// 이번에는 버튼이 많아서 보기 편하게 관련된 위젯과 붙여놨습니다

	// 패널
	UPROPERTY(meta = (BindWidget))
	class UPreDeployPanelBase* _primaryEquipPanel;
	UPROPERTY(meta = (BindWidget))
	class UPreDeployPanelBase* _secondaryEquipPanel;
	UPROPERTY(meta = (BindWidget))
	class UPreDeployStratagemPanel* _stratagemPanel;


	// Launch 버튼
	UFUNCTION()
	void HandleLaunchRequest();

	UPROPERTY(meta = (BindWidget))
	class UButton* _launchBtn;


	// 패널 전환 (장비 <-> 스트라타젬)
	UFUNCTION()
	void SwitchToEquipPage();
	UFUNCTION()
	void SwitchToStratagemPage();

	void OpenStratagemPanel(bool isOpened);

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* _pageSwitcher;

	UPROPERTY(meta = (BindWidget))
	class UButton* _equipBtn; // 스트라타젬에서 장비로
	UPROPERTY(meta = (BindWidget))
	class UButton* _stgBtn; // 장비에서 스트라타젬으로


	// 선택창 <-> 무기 패널
	UFUNCTION()
	void OpenPrimaryEquipPanel();
	UFUNCTION()
	void OpenSecondaryPanel();
	UFUNCTION()
	void ReturnToEquipPage();

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* _hubSwitcher;

	UPROPERTY(meta = (BindWidget))
	class UButton* _primaryGunBtn;
	UPROPERTY(meta = (BindWidget))
	class UButton* _secondaryGunBtn;


	UFUNCTION()
	void OnESCPressed();

	UPROPERTY(meta = (BindWidget))
	class UButton* _escBtn;	// 버튼으로 임시 구현
};
