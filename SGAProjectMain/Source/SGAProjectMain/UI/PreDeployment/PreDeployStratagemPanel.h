// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PreDeployPanelBase.h"
#include "PreDeployStratagemPanel.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UPreDeployStratagemPanel : public UPreDeployPanelBase
{
	GENERATED_BODY()
	
public:
	virtual void InitializePanel(class UPreDeploymentState* state) override;

	virtual void HandleEntryPicked(UPreDeployEntryBase* entry) override;
	void HandleSlotPicked(UPreDeployEntryBase* slot, int32 slotIndex);

protected:
	void SelectSlot(int32 slotIndex);
	int32 FindEmptySlotIndex();

	void ClosePanel();

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* _slotPanel;

	UPROPERTY()
	TArray<UPreDeployEntryBase*> _selectedStgEntries; // 선택된 스트라타젬 엔트라

	int32 _curSlotIndex = -1;
};
