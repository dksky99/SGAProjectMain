// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Data/ShopItemTable.h"
#include "ShopWidgetBase.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UShopWidgetBase : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	void SetPlayerCurrencyDisplay(FPlayerCurrency _currency);
	void OnSlotPicked(int32 itemID);
	void PurchaseItem();

public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _requisitionSlipText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _medalText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _commonSampleText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _rareSampleText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _superSampleText;

	UPROPERTY(meta = (BindWidget))
	class UButton* _purchaseButton;

	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* _slotPanel;

	UPROPERTY(EditAnywhere, Category = "Game/UI")
	TSubclassOf<class UShopSlotWidgetBase> _slotClass;

	UPROPERTY(EditAnywhere, Category = "Game/Shop")
	EShopType _shopType = EShopType::None;

	TArray<class UShopSlotWidgetBase*> _shopSlots;

	int32 _selectedItemID = -1;
};
