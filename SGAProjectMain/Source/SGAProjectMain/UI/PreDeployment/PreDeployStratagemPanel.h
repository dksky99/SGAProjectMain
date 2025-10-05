// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PreDeployPanelBase.h"
#include "PreDeployStratagemPanel.generated.h"

/**
 * 
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPanelOpened, bool);

UCLASS()
class SGAPROJECTMAIN_API UPreDeployStratagemPanel : public UPreDeployPanelBase
{
	GENERATED_BODY()
	
public:
	void InitializePanel(class UPreDeploymentState* state) override;

	void HandleEntryPicked(UPreDeployEntryBase* entry) override;
	void HandleSlotPicked(UPreDeployEntryBase* slot, int32 slotIndex);

	FOnPanelOpened _panelOpenedEvent;

	bool IsPanelOpen() const { return _isPanelOpen; }

	void OpenPanel();
	void ClosePanel();

protected:
	void SelectSlot(int32 slotIndex);
	int32 FindEmptySlotIndex();

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* _slotPanel;

	UPROPERTY(meta = (BindWidget))
	class UBorder* _backgroundBorder;

	UPROPERTY()
	TArray<UPreDeployEntryBase*> _selectedStgEntries; // 선택된 스트라타젬 엔트리

	int32 _curSlotIndex = -1;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* _slotPanelUp;

	bool _isPanelOpen = false;
};
