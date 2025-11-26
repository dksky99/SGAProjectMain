// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopWidgetBase.h"

#include "Components/Button.h"
#include "../CGameInstance.h"
#include "ShopSlotWidgetBase.h"

void UShopWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

    UCGameInstance* GI = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
    UDataTable* table = GI->GetShopItemTable();

    _slotPanel->ClearChildren();

    for (auto& row : table->GetRowMap())
    {
        FShopItemData* itemData = (FShopItemData*)row.Value;
        if (itemData->_shopType == _shopType)
        {
            UShopSlotWidgetBase* slot = CreateWidget<UShopSlotWidgetBase>(this, _slotClass);
			int32 itemID = itemData->_itemID;
            //slot->InitializeSlot(itemID);
            slot->_slotPickedEvent.AddUObject(this, &UShopWidgetBase::OnSlotPicked);
            _slotPanel->AddChild(slot);
        }
	}

	_purchaseButton->OnClicked.AddDynamic(this, &UShopWidgetBase::PurchaseItem);
}

void UShopWidgetBase::OnSlotPicked(int32 itemID)
{
	_selectedItemID = itemID;
}

void UShopWidgetBase::PurchaseItem()
{
	if (_selectedItemID == -1)
		return;

	auto GI = GetWorld()->GetGameInstance<UCGameInstance>();
	GI->TryPurchaseShopItem(_shopType, _selectedItemID);
}
