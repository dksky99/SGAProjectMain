// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Data/MissionResult.h"
#include "Data/ShopItemTable.h"
#include "Object/AbnormalityTable.h"
#include "Character/UnitDataTable.h"
#include "Character/StatComponent.h"
#include "CGameInstance.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FProcessedUnitData
{
	GENERATED_BODY()

	// FUnitData에서 직접 복사되는 기본 정보
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UnitData")
	FName _name;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UnitData")
	float _defaultMovementSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UnitData")
	float _battleMovementSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UnitData")
	FUnitAbnormalResistData _resistData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UnitData")
	TSubclassOf<class ACharacterBase> _unitClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UnitData")
	FText _desc;

	// FUnitData의 PartID가 FPartDefinitionRow 데이터로 변환되어 저장되는 부분
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UnitData|Parts")
	TMap<EBodyPart,struct FUnitPartStatArrayWrapper> _partDatas; // _main_PartID가 변환된 데이터

	// 기본 생성자
	FProcessedUnitData()
		: _name(FName())
		, _defaultMovementSpeed(0.0f)
		, _battleMovementSpeed(0.0f)
		, _unitClass(nullptr)
		, _desc(FText::GetEmpty())
	{
		// _resistData, _..._Part 변수들은 기본 생성자로 자동 초기화됩니다.
	}
};


USTRUCT(BlueprintType)
struct FProcessedAbnormalityDefinitionData
{
	GENERATED_BODY()


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _statusID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName _name;

	//지속시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _stateDruration = 0.f;

	//부여될 가중치.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _stateWeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAbnormalityType _abnormalityType = EAbnormalityType::None;


};


UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	None,
	Lobby,
	InMission
};

USTRUCT(BlueprintType)
struct FSelectedStratagemSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<class AStratagem>> SelectedStratagems;
};

UCLASS()
class SGAPROJECTMAIN_API UCGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UCGameInstance();
	virtual void Init() override;

	UPROPERTY(BlueprintReadWrite)
	TMap<FName, FSelectedStratagemSet> AllPlayerStratagemSets;

	//장비
	struct FGunData GetGunDataFromTable(int32 id);
	TSubclassOf<class AGunBase> GetGunClassFromTable(int32 id);
	class UTexture2D* GetGunPreviewFromTable(int32 id);
	class UDataTable* GetGunTable() { return _gunTable; }




	//스트라타젬
	struct FStratagemSlot GetStratagemSlotFromTable(int32 id);
	TSubclassOf<class AStratagem> GetStratagemClassFromTable(int32 id);
	class UDataTable* GetStratagemTable() { return _stratagemTable; }

	// 재화
	void AddRewardCurrency(const FPlayerCurrency& reward);
	FSampleBundle GetSavedSample();
	FPlayerCurrency GetCurrentCurrency();

	// 임무 및 미션
	class UOperationDataAsset* GetOperationDataAsset(FName operationID);
	class UMissionDataAsset* GetMissionDataAsset(FName missionID);
	void SetOperationAndMission(UOperationDataAsset* operation, UMissionDataAsset* mission);
	void ApplyMissionResult(const FMissionResult& missionResult);

	//유닛
	struct FUnitData GetUnitDataFromTable(int32 id);
	TSubclassOf<class ACharacterBase> GetUnitClassFromTable(int32 id);
	//스테이터스 정보 반환받기
	const FProcessedUnitData* GetProcessedUnitData(TSubclassOf<class ACharacterBase> UnitID);

	struct FUnitPartStatArrayWrapper GetPartDataFromTable(FName partID);
	class UDataTable* GetUnitTable() { return _unitTable; }

	//상태이상
	class UDataTable* GetAbnormalityTable() { return _abnormalityTable; }
	struct FProcessedAbnormalityDefinitionData GetAbnormalDefinitionDataFromTable(FName partID);
	const FProcessedAbnormalityDefinitionData* GetProcessedAbnormalityDefinitionData(EAbnormality state);
	const struct FCDamageEvent* GetAbnormalDamageEventData(EAbnormality state);

	// 세이브
	void LoadGame();
	void SaveGame();
	class UCSaveGame* GetCurrentSave() { return _curSaveGame; }

	// 상점
	bool IsShopItemPurchased(EShopType type, int32 id);
	bool IsShopItemUnlockConditionMet(int32 condition);
	bool CanAffordShopItem(FPlayerCurrency price);
	bool TryPurchaseShopItem(EShopType type, int32 id);

	struct FShopItemData GetShopItemByID(EShopType type, int32 id);
	class UDataTable* GetShopItemTable() { return _shopItemTable; }
	struct FPlayerCurrency GetShopItemPriceByID(EShopType type, int32 id);

	class UPreDeploymentState* GetPreDeployState() { return _preDeployState; }

	void SetGamePhase(EGamePhase newPhase) { _curGamePhase = newPhase; }
	EGamePhase GetGamePhase() const { return _curGamePhase; }


	void InitializeUnitData();


protected:
	//미리 데이터들을 캐싱
	void CachingAbnormalityDataFromTable();
	void CachingUnitDataFromTable();
	void CachingOperationAndMissionData();

private:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UDataTable* _gunTable;
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UDataTable* _stratagemTable;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TArray<class UOperationDataAsset*> _operations;
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TArray<class UMissionDataAsset*> _missions;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Data", meta = (AllowPrivateAccess = "true"))
	TMap<FName, UOperationDataAsset*> _operationMap;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Data", meta = (AllowPrivateAccess = "true"))
	TMap<FName, UMissionDataAsset*> _missionMap;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UDataTable* _unitTable;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UDataTable* _unitPartDefinitionTable;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Data", meta = (AllowPrivateAccess = "true"))
	TMap<TSubclassOf<class ACharacterBase>, FProcessedUnitData> _processedUnitDataMap;


	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UDataTable* _abnormalityTable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Data", meta = (AllowPrivateAccess = "true"))
	TMap<EAbnormality, FProcessedAbnormalityDefinitionData> _abnormalityDefinitions;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Data", meta = (AllowPrivateAccess = "true"))
	TMap<EAbnormality, struct FCDamageEvent> _abnormalityDamageEvents;

	UPROPERTY()
	class UCSaveGame* _curSaveGame = nullptr;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UDataTable* _shopItemTable;

	UPROPERTY()
	UPreDeploymentState* _preDeployState;

	EGamePhase _curGamePhase = EGamePhase::None;
};
