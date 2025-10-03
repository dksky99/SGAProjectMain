// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/PanelWidget.h"
#include "PreDeployEntryBase.h"
#include "PreDeployCategorySection.h"
#include "../../Game/PreDeployment/PreDeploymentState.h"
#include "../../Gun/GunDataTable.h"
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

	UPROPERTY(EditAnywhere, Category = "Game/UI")
	EGunSlotType _panelSlotType = EGunSlotType::Primary;

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
	UPreDeployEntryBase* _curSelectedEntry;

	int32 _curSectionIndex = 0;
	int32 _curRow = 0;
	int32 _curCol = 0;

private:
	UFUNCTION()
	void HandleEquipRequest();

	int32 GetColumnNumInRow(int32 sec, int32 row);
	int32 GetRowNumInSection(int32 sec);

	void SelectEntry(int32 sec, int32 row, int32 col);

	UPROPERTY(meta = (BindWidget))
	class UButton* _equipBtn;
};
