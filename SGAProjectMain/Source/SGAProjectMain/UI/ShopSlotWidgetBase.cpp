// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopSlotWidgetBase.h"

#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "../Data/ShopItemTable.h"

void UShopSlotWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	_button->OnClicked.AddDynamic(this, &UShopSlotWidgetBase::HandlePick);
}

void UShopSlotWidgetBase::HandlePick()
{
	if (_slotPickedEvent.IsBound())
		_slotPickedEvent.Broadcast(_itemID);
}

void UShopSlotWidgetBase::InitializeSlot(FShopItemData* itemData)
{
//	_itemID = itemData->_itemID;




}
