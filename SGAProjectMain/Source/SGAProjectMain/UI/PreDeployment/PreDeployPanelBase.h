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
	virtual void InitializePanel(class UPreDeploymentState* state);

	virtual void HandleEntryPicked(UPreDeployEntryBase* entry);

	FOnSelectChanged _selectChangedEvent;
	
protected:
	void OnEntrySpawned(UPreDeployEntryBase* entry);

	UPROPERTY(EditAnywhere, Category = "Game/UI")
	TSubclassOf<class UPreDeployCategorySection> _categoryClass;

	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* _sectionPanel;

	UPROPERTY()
	class UPreDeploymentState* _state;

	UPROPERTY()
	UPreDeployEntryBase* _curSelectedEntry;

	UPROPERTY(EditAnywhere, Category = "Game/UI")
	EGunSlotType _panelSlotType = EGunSlotType::Primary;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _curSectionText;

	UPROPERTY(meta = (BindWidget))
	class UPreDeployDetailBase* _detailPanel;

private:
	UFUNCTION()
	void HandleEquipRequest();

	UPROPERTY(meta = (BindWidget))
	class UButton* _equipBtn;
};
