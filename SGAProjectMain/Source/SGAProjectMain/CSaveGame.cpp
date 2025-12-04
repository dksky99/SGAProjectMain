// Fill out your copyright notice in the Description page of Project Settings.


#include "CSaveGame.h"

void UCSaveGame::AddCurrency(const FPlayerCurrency& currency)
{
	_playerCurrency.AddCurrency(currency);
}

void UCSaveGame::SubtractCurrency(const FPlayerCurrency& currency)
{
	if (_playerCurrency.CanAfford(currency))
		_playerCurrency.SubtractCurrency(currency);
}

void UCSaveGame::ResetOperationData()
{
	_curOperationID = FName();
	_completedMissionIDs.Empty();
}

void UCSaveGame::SetCurOperationID(FName operationID)
{
	if (operationID == _curOperationID)
		return;

	// 현재 진행 중인 임무 ID가 변경되었을 경우에만 갱신
	ResetOperationData();
	_curOperationID = operationID;
}

void UCSaveGame::AddPurchasedShopItem(EShopType type, int32 id)
{
	FOwnedItem newItem;
	newItem._type = type;
	newItem._id = id;
	_purchasedShopItems.Add(newItem);
}

bool UCSaveGame::IsShopItemPurchased(EShopType type, int32 id)
{
	FOwnedItem item;
	item._type = type;
	item._id = id;
	return _purchasedShopItems.Contains(item);
}
