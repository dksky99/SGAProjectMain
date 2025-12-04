// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Data/PlayerCurrency.h"
#include "Data/ShopItemTable.h"
#include "CSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UCSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	void AddCurrency(const FPlayerCurrency& currency);
	void SubtractCurrency(const FPlayerCurrency& currency);
	const FPlayerCurrency& GetPlayerCurrency() { return _playerCurrency; }

	void ResetOperationData();
	void SetCurOperationID(FName operationID);
	void AddCompletedMissionID(FName missionID) { _completedMissionIDs.Add(missionID); }

	FName GetCurOperationID() const { return _curOperationID; }
	TSet<FName>& GetCompletedMissionIDs() { return _completedMissionIDs; }

	void AddPurchasedShopItem(EShopType type, int32 id);
	bool IsShopItemPurchased(EShopType type, int32 id);
	TArray<struct FOwnedItem>& GetPurchasedShopItems() { return _purchasedShopItems; }

	const int32 GetPlayerLevel() const { return _playerLevel; }
	const int32 GetPlayerExperience() const { return _playerExp; }
	void SetPlayerLevel(int32 newLevel) { _playerLevel = newLevel; }
	void SetPlayerExperience(int32 newExp) { _playerExp = newExp; }

protected:
	UPROPERTY(VisibleAnywhere, Category = "Game/SaveGame")
	int32 _playerLevel; // 플레이어 레벨
	UPROPERTY(VisibleAnywhere, Category = "Game/SaveGame")
	int32 _playerExp; // 플레이어 경험치

	UPROPERTY(VisibleAnywhere, Category = "Game/SaveGame")
	FPlayerCurrency _playerCurrency; // 플레이어가 보유한 화폐 정보

	UPROPERTY(VisibleAnywhere, Category = "Game/SaveGame")
	FName _curOperationID; // 현재 진행 중인 임무 ID

	UPROPERTY(VisibleAnywhere, Category = "Game/SaveGame")
	TSet<FName> _completedMissionIDs; // _curOperation에 속한 미션 중 완료한 미션 목록

	UPROPERTY(VisibleAnywhere, Category = "Game/SaveGame")
	TArray<struct FOwnedItem> _purchasedShopItems; // 구매한 상점 아이템 ID 목록
};
