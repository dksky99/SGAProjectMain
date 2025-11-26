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
	void OnSlotPicked(int32 itemID);
	void PurchaseItem();

public:
	UPROPERTY(meta = (BindWidget))
	class UButton* _purchaseButton;

	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* _slotPanel;

	UPROPERTY(EditAnywhere, Category = "Game/UI")
	TSubclassOf<class UShopSlotWidgetBase> _slotClass;

	UPROPERTY(EditAnywhere, Category = "Game/Shop")
	EShopType _shopType = EShopType::None;

	int32 _selectedItemID = -1;
};
