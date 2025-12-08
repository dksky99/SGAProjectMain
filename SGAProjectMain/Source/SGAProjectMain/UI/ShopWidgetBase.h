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
	virtual void InitializeWidget();
	void SetPlayerCurrencyDisplay(FPlayerCurrency _currency);
	void OnSlotPicked(class UShopSlotWidgetBase* slot);

	UFUNCTION()
	void PurchaseItem();

	UFUNCTION()
	void OnESC(const FInputActionValue& value);

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
	class UBorder* _buttonBorder;
	UPROPERTY(meta = (BindWidget))
	class UButton* _purchaseButton;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _buttonText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _priceText;

	UPROPERTY(meta = (BindWidget))
	class UScrollBox* _scrollBox;
	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* _stgSlotPanel;
	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* _gunSlotPanel;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _slotText;

	UPROPERTY(meta = (BindWidget))
	class UPreDeployDetailBase* _stgDetailPanel;
	UPROPERTY(meta = (BindWidget))
	class UPreDeployDetailBase* _gunDetailPanel;


	UPROPERTY(EditAnywhere, Category = "Game/UI")
	TSubclassOf<class UShopSlotWidgetBase> _slotClass;

	//UPROPERTY(EditAnywhere, Category = "Game/Shop")
	//EShopType _shopType = EShopType::None;

	TArray<class UShopSlotWidgetBase*> _shopSlots;

	class UShopSlotWidgetBase* _selectedSlot = nullptr;
	int32 _selectedItemID = -1;

	UPROPERTY(EditAnywhere, Category = "Game/Input")
	class UInputAction* _escAction;

	UPROPERTY(EditAnywhere, Category = "Game/IMC")
	class UInputMappingContext* _gameIMC;
	UPROPERTY(EditAnywhere, Category = "Game/IMC")
	class UInputMappingContext* _widgetIMC;
};
