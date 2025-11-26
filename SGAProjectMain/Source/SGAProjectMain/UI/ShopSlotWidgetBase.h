// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopSlotWidgetBase.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSlotPicked, int32 id);
/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UShopSlotWidgetBase : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandlePick();

public:
	void InitializeSlot(struct FShopItemData* itemData);

	FOnSlotPicked _slotPickedEvent;

protected:
	UPROPERTY(meta = (BindWidget))
	class UButton* _button;

	UPROPERTY(meta = (BindWidget))
	class UBorder* _border;

	UPROPERTY(meta = (BindWidget))
	class UImage* _itemImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* _equipMark;

	UPROPERTY(EditAnywhere)
	int32 _itemID;
};
