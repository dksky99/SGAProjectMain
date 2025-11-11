// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Object/Item/SampleResources.h"

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
	FText _name;

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
		: _name(FText::GetEmpty())
		, _defaultMovementSpeed(0.0f)
		, _battleMovementSpeed(0.0f)
		, _unitClass(nullptr)
		, _desc(FText::GetEmpty())
	{
		// _resistData, _..._Part 변수들은 기본 생성자로 자동 초기화됩니다.
	}
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

	struct FGunData GetGunDataFromTable(int32 id);
	TSubclassOf<class AGunBase> GetGunClassFromTable(int32 id);
	class UTexture2D* GetGunPreviewFromTable(int32 id);
	class UDataTable* GetGunTable() { return _gunTable; }


	struct FUnitData GetUnitDataFromTable(int32 id);
	TSubclassOf<class ACharacterBase> GetUnitClassFromTable(int32 id);
	struct FUnitPartStatArrayWrapper GetPartDataFromTable(FName partID);
	class UDataTable* GetUnitTable() { return _unitTable; }

	struct FStratagemSlot GetStratagemSlotFromTable(int32 id);
	TSubclassOf<class AStratagem> GetStratagemClassFromTable(int32 id);
	class UDataTable* GetStratagemTable() { return _stratagemTable; }

	void AddEarnedSample(const FSampleBundle& earnedSample);
	FSampleBundle GetSavedSample() { return _savedSample; }


	class UPreDeploymentState* GetPreDeployState() { return _preDeployState; }

	void SetGamePhase(EGamePhase newPhase) { _curGamePhase = newPhase; }
	EGamePhase GetGamePhase() const { return _curGamePhase; }


	void InitializeUnitData();

	UFUNCTION(BlueprintCallable, Category = "Game Data")
	bool GetProcessedUnitData(FName UnitID, FProcessedUnitData& OutData);

private:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UDataTable* _gunTable;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UDataTable* _stratagemTable;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UDataTable* _unitTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data Tables", meta = (AllowPrivateAccess = "true"))
	class UDataTable* _unitPartDefinitionTable;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Data", meta = (AllowPrivateAccess = "true"))
	TMap<FName, FProcessedUnitData> ProcessedUnitDataMap;


	UPROPERTY(EditAnywhere, Category = "Sample")
	FSampleBundle _savedSample;

	UPROPERTY()
	UPreDeploymentState* _preDeployState;

	EGamePhase _curGamePhase = EGamePhase::None;

	

};
