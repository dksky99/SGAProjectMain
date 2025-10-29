// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PreDeployPanelBase.h"
#include "../../Data/GunDataTable.h"
#include "PreDeployWeaponPanel.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UPreDeployWeaponPanel : public UPreDeployPanelBase
{
	GENERATED_BODY()

public:
	void InitializePanel(UPreDeploymentState* state) override;

protected:
	FText GetSectionText(UPreDeployEntryBase* entry) override;
	void FocusEntry(UPreDeployEntryBase* entry) override;
	void OnEntrySpawned(UPreDeployEntryBase* entry) override;

	UPROPERTY(EditAnywhere, Category = "Game/UI")
	EGunSlotType _panelSlotType = EGunSlotType::Primary;

	UFUNCTION()
	void HandleEquipRequest();

	UPROPERTY(meta = (BindWidget))
	class UButton* _equipBtn;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _equipText;
	UPROPERTY(meta = (BindWidget))
	class UBorder* _equipBorder;
};
