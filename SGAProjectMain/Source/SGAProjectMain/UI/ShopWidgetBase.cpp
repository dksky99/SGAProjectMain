// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopWidgetBase.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
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
            slot->InitializeSlot(*itemData);
            slot->_slotPickedEvent.AddUObject(this, &UShopWidgetBase::OnSlotPicked);
            _slotPanel->AddChild(slot);
			_shopSlots.Add(slot);
        }
	}

	SetPlayerCurrencyDisplay(GI->GetCurrentCurrency());

	_purchaseButton->OnClicked.AddDynamic(this, &UShopWidgetBase::PurchaseItem);
}

void UShopWidgetBase::SetPlayerCurrencyDisplay(FPlayerCurrency currency)
{
    _requisitionSlipText->SetText(FText::AsNumber(currency._requisitionSlips));
    _medalText->SetText(FText::AsNumber(currency._medals));
    _commonSampleText->SetText(FText::AsNumber(currency.GetSampleCount(ESampleType::Common)));
    _rareSampleText->SetText(FText::AsNumber(currency.GetSampleCount(ESampleType::Rare)));
	_superSampleText->SetText(FText::AsNumber(currency.GetSampleCount(ESampleType::Super)));
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
	bool success = GI->TryPurchaseShopItem(_shopType, _selectedItemID);

    if (success)
    {
        // 구매 성공 시 슬롯 상태 갱신
        for (auto& slot : _shopSlots)
        {
            auto data = GI->GetShopItemByID(slot->GetItemID());
			slot->InitializeSlot(data);
		}
		// 플레이어 재화 표시 갱신
		SetPlayerCurrencyDisplay(GI->GetCurrentCurrency());
    }
}
