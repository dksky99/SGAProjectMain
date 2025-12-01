// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopSlotWidgetBase.h"

#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "../Data/ShopItemTable.h"
#include "../CGameInstance.h"

void UShopSlotWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	_button->OnClicked.AddDynamic(this, &UShopSlotWidgetBase::HandlePick);
}

void UShopSlotWidgetBase::HandlePick()
{
	if (_slotPickedEvent.IsBound())
		_slotPickedEvent.Broadcast(this);
}

void UShopSlotWidgetBase::InitializeSlot(const FShopItemData& itemData)
{
	_itemID = itemData._itemID;
	_shopType = itemData._shopType;

	if (_itemImage)
		_itemImage->SetBrushFromTexture(itemData._itemImage);

	if (_text)
	{
		//if (itemData._shopType == EShopType::Gun) // 총일 경우 가격을 메달로 표시
		//{
		//	FString priceString = FString::Printf(TEXT("Price: %d"), itemData._price._medals);
		//	_text->SetText(FText::FromString(priceString));
		//}
		//if (itemData._shopType == EShopType::Stratagem) // 스트라타젬일 경우 이름 표시
		{
			_text->SetText(FText::FromString(itemData._itemName));
		}
	}

	auto GI = GetWorld()->GetGameInstance<UCGameInstance>();
	if (!GI) return;

	if (GI->IsShopItemPurchased(itemData._shopType, itemData._itemID))
	{
		SetPurchased(); // 이미 구매했을 경우
		return;
	}

	if (!GI->IsShopItemUnlockConditionMet(itemData._condition))
	{
		SetLocked(); // 잠금 조건이 충족되지 않았을 경우
		return;
	}

	if (GI->CanAffordShopItem(itemData._price))
	{
		SetAffordable(); // 구매 가능
	}
	else
	{
		SetUnaffordable(); // 구매 불가
	}
}

void UShopSlotWidgetBase::SetSelected(bool selected)
{
	FLinearColor color = selected ? FLinearColor::Yellow : FLinearColor::White;
	_border->SetBrushColor(color);
	float targetOpacity = selected ? 1.0f : _selectedOpacity;
	this->SetRenderOpacity(targetOpacity);
}

void UShopSlotWidgetBase::SetPurchased()
{
	this->SetRenderOpacity(0.8f);
	_selectedOpacity = 0.8f;
	if (_equipMark && _purchasedImage)
		_equipMark->SetBrushFromTexture(_purchasedImage);
}

void UShopSlotWidgetBase::SetLocked()
{
	this->SetRenderOpacity(0.5f);
	_selectedOpacity = 0.5f;
	if (_equipMark && _lockedImage)
		_equipMark->SetBrushFromTexture(_lockedImage);
}

void UShopSlotWidgetBase::SetAffordable()
{
	this->SetRenderOpacity(1.0f);
	_selectedOpacity = 1.0f;
	if (_equipMark && _affordableImage)
		_equipMark->SetBrushFromTexture(_affordableImage);
}

void UShopSlotWidgetBase::SetUnaffordable()
{
	this->SetRenderOpacity(0.5f);
	_selectedOpacity = 0.5f;
	if (_equipMark)
		_equipMark->SetVisibility(ESlateVisibility::Hidden);
}
