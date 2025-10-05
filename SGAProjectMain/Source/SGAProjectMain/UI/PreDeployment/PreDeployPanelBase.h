// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/PanelWidget.h"
#include "PreDeployEntryBase.h"
#include "PreDeployCategorySection.h"
#include "../../Game/PreDeployment/PreDeploymentState.h"
#include "../../CGameInstance.h"
#include "PreDeployPanelBase.generated.h"

/**
 * 
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSelectChanged, int32);

UCLASS()
class SGAPROJECTMAIN_API UPreDeployPanelBase : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void InitializePanel(UPreDeploymentState* state);

	virtual void HandleEntryPicked(UPreDeployEntryBase* entry);

	FOnSelectChanged _selectChangedEvent;

	virtual void MoveLeft();
	virtual	void MoveRight();
	virtual void MoveUp();
	virtual void MoveDown();
	
protected:
	void OnEntrySpawned(UPreDeployEntryBase* entry);

	UPROPERTY(EditAnywhere, Category = "Game/UI")
	TSubclassOf<UPreDeployCategorySection> _categoryClass;

	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* _sectionPanel;
	UPROPERTY(meta = (BindWidget))
	class UPreDeployDetailBase* _detailPanel;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _curSectionText;

	UPROPERTY()
	class UPreDeploymentState* _state;

	UPROPERTY()
	TArray<UPreDeployCategorySection*> _sections;

	UPROPERTY()
	UPreDeployEntryBase* _curSelectedEntry; // 현재 선택한 엔트리(디테일 표시)
	UPROPERTY()
	UPreDeployEntryBase* _lastEquippedEntry; // 현재 장착된 엔트리 중 마지막으로 장착된 엔트리

	int32 _curSectionIndex = 0;
	int32 _curRow = 0;
	int32 _curCol = 0;

private:
	int32 GetColumnNumInRow(int32 sec, int32 row);
	int32 GetRowNumInSection(int32 sec);

	void SelectEntry(int32 sec, int32 row, int32 col);
};
