// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Gun/GunBase.h"
#include "GunSettingWidget.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UGunSettingWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitializeWidget(class AGunBase* gun);

	void UpdateGunInfo(const struct FGunData& gunData, int32 curAmmo);
	void SetSlots(const FGunData& gunData);
	void AddSlotToPanel(UPanelWidget* panel, class UGunSettingSlotDataAsset* asset);

	void UpdateFireModePanel(EFireMode fireMode);
	void UpdateLightModePanel(ETacticalLightMode lightMode); 
	void UpdateScopeModePanel(int32 scopeMode);

	void HighlightSlot(UPanelWidget* Panel, FName Key);

	class UGunSettingSlotDataAsset* FindSlotAsset(FName key);

private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _gunNameText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _curAmmoText;

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* _fireModePanel;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* _scopeModePanel;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* _lightModePanel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/SlotData", meta = (AllowPrivateAccess = "true"))
	TArray<class UGunSettingSlotDataAsset*> _gunSlotDataAssets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UGunSettingSlotWidget> _slotWidgetClass;

	UEnum* fireEnum = StaticEnum<EFireMode>();
	UEnum* lightEnum = StaticEnum<ETacticalLightMode>();
};
