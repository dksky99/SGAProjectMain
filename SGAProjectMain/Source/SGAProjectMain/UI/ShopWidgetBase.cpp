// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopWidgetBase.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/Border.h"
#include "../CGameInstance.h"
#include "ShopSlotWidgetBase.h"
#include "PreDeployment/PreDeployDetailBase.h"

void UShopWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

    InitializeWidget();

	_purchaseButton->OnClicked.AddDynamic(this, &UShopWidgetBase::PurchaseItem);

    if (auto* EIC = Cast<UEnhancedInputComponent>(GetOwningPlayer()->InputComponent))
    {
        EIC->BindAction(_escAction, ETriggerEvent::Started, this, &UShopWidgetBase::OnESC);
    }
}

void UShopWidgetBase::InitializeWidget()
{
    UCGameInstance* GI = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
    UDataTable* table = GI->GetShopItemTable();

    _gunSlotPanel->ClearChildren();
	_stgSlotPanel->ClearChildren();

    for (auto& row : table->GetRowMap())
    {
        FShopItemData* itemData = (FShopItemData*)row.Value;
        //if (itemData->_shopType == _shopType)
        {
            UShopSlotWidgetBase* slot = CreateWidget<UShopSlotWidgetBase>(this, _slotClass);
            int32 itemID = itemData->_itemID;
            slot->InitializeSlot(*itemData);
            slot->_slotPickedEvent.AddUObject(this, &UShopWidgetBase::OnSlotPicked);

            if (itemData->_shopType == EShopType::Stratagem)
                _stgSlotPanel->AddChild(slot);
            else if (itemData->_shopType == EShopType::Gun)
				_gunSlotPanel->AddChild(slot);
            _shopSlots.Add(slot);
        }
    }

    SetPlayerCurrencyDisplay(GI->GetCurrentCurrency());

    _stgDetailPanel->SetVisibility(ESlateVisibility::Collapsed);
	_gunDetailPanel->SetVisibility(ESlateVisibility::Collapsed);

    OnSlotPicked(_shopSlots[0]);
}

void UShopWidgetBase::SetPlayerCurrencyDisplay(FPlayerCurrency currency)
{
    _requisitionSlipText->SetText(FText::AsNumber(currency._requisitionSlips));
    _medalText->SetText(FText::AsNumber(currency._medals));
    _commonSampleText->SetText(FText::AsNumber(currency.GetSampleCount(ESampleType::Common)));
    _rareSampleText->SetText(FText::AsNumber(currency.GetSampleCount(ESampleType::Rare)));
	_superSampleText->SetText(FText::AsNumber(currency.GetSampleCount(ESampleType::Super)));
}

void UShopWidgetBase::OnSlotPicked(UShopSlotWidgetBase* slot)
{
	// 이전에 선택된 슬롯이 있으면 선택 해제
    if (_selectedSlot)
        _selectedSlot->SetSelected(false);

    if (!slot)
    {
        _selectedSlot = nullptr;
        _selectedItemID = -1;
        _gunDetailPanel->SetVisibility(ESlateVisibility::Collapsed);
		_stgDetailPanel->SetVisibility(ESlateVisibility::Collapsed);
        return;
    }
        
	// 새로 선택된 슬롯 설정
    _selectedSlot = slot;
    _selectedItemID = slot->GetItemID();
    slot->SetSelected(true);

	EShopType shopType = slot->GetShopType();

    if (shopType == EShopType::Gun)
    {
        _slotText->SetText(FText::FromString(TEXT("Gun")));
        _gunDetailPanel->SetVisibility(ESlateVisibility::Visible);
        _stgDetailPanel->SetVisibility(ESlateVisibility::Collapsed);
		_gunDetailPanel->SetDetail(_selectedItemID);
    }
    else if (shopType == EShopType::Stratagem)
    {
        _slotText->SetText(FText::FromString(TEXT("Stratagem")));
        _stgDetailPanel->SetVisibility(ESlateVisibility::Visible);
        _gunDetailPanel->SetVisibility(ESlateVisibility::Collapsed);
		_stgDetailPanel->SetDetail(_selectedItemID);
    }
    else
    {
        return;
	}

    if (_scrollBox)
    {
        _scrollBox->ScrollWidgetIntoView(
            slot,
            true,
            EDescendantScrollDestination::IntoView,
            0.15f
        );
    }

	auto GI = GetWorld()->GetGameInstance<UCGameInstance>();
    if (!GI) return;

	FPlayerCurrency price = GI->GetShopItemPriceByID(shopType, _selectedItemID);
    if (shopType == EShopType::Stratagem)
    {
        FString priceString = FString::Printf(TEXT("%d"), price._requisitionSlips);
        _priceText->SetText(FText::FromString(priceString));
    }
    else if (shopType == EShopType::Gun)
    {
        FString priceString = FString::Printf(TEXT("%d"), price._requisitionSlips);
        _priceText->SetText(FText::FromString(priceString));
	}

    if (GI->IsShopItemPurchased(shopType, _selectedItemID))
    {
        _buttonBorder->SetBrushColor(FLinearColor::Green);
        _buttonText->SetText(FText::FromString(TEXT("OWNED")));
		_buttonText->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
        _purchaseButton->SetIsEnabled(false);
        return;
	}

    if (!GI->IsShopItemUnlockConditionMet(GI->GetShopItemByID(shopType, _selectedItemID)._condition))
    {
        _buttonBorder->SetBrushColor(FLinearColor::Red);
        _buttonText->SetText(FText::FromString(TEXT("LOCKED")));
		_buttonText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
        _purchaseButton->SetIsEnabled(false);
		return;
    }

    if (GI->CanAffordShopItem(price))
    {
        _buttonBorder->SetBrushColor(FLinearColor::Yellow);
        _buttonText->SetText(FText::FromString(TEXT("PURCHASE")));
		_buttonText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
        _purchaseButton->SetIsEnabled(true);
    }
    else
    {
        _buttonBorder->SetBrushColor(FLinearColor::Red);
        _buttonText->SetText(FText::FromString(TEXT("LOW FUNDS")));
		_buttonText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
        _purchaseButton->SetIsEnabled(false);
	}
}

void UShopWidgetBase::PurchaseItem()
{
	if (_selectedItemID == -1)
		return;

	auto GI = GetWorld()->GetGameInstance<UCGameInstance>();
	if (!GI) return;

	EShopType shopType = _selectedSlot->GetShopType();
	bool success = GI->TryPurchaseShopItem(shopType, _selectedItemID);

    if (success)
    {
        // 구매 성공 시 슬롯 상태 갱신
        for (auto& slot : _shopSlots)
        {
			shopType = slot->GetShopType();
            auto data = GI->GetShopItemByID(shopType, slot->GetItemID());
			slot->InitializeSlot(data);
			// 선택된 슬롯이면 다시 선택 상태로 설정
            if (slot == _selectedSlot)
                slot->SetSelected(true);
			
		}
		// 플레이어 재화 표시 갱신
		SetPlayerCurrencyDisplay(GI->GetCurrentCurrency());
    }
}

void UShopWidgetBase::OnESC(const FInputActionValue& value)
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    if (auto* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
    { // IMC 교체
        subsystem->RemoveMappingContext(_widgetIMC);
        subsystem->AddMappingContext(_gameIMC, 0);
    }

    FInputModeGameOnly mode;
    PC->SetInputMode(mode);
    PC->bShowMouseCursor = false;

    RemoveFromParent();
}
