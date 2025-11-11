// Fill out your copyright notice in the Description page of Project Settings.


#include "CGameInstance.h"

#include "Kismet/GameplayStatics.h"

#include "Game/PreDeployment/PreDeploymentState.h"
#include "Data/GunDataTable.h"
#include "Gun/GunBase.h"

#include "StratagemComponent.h"
#include "Object/Stratagem/Stratagem.h"

#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"


#include "Character/CharacterBase.h"
#include "Character/StatComponent.h"

void UCGameInstance::Init()
{
	Super::Init();

	_preDeployState = NewObject<UPreDeploymentState>(this);

	//유닛 데이터 초기화함수.
	//InitializeUnitData();
}


UCGameInstance::UCGameInstance()
{



}

FGunData UCGameInstance::GetGunDataFromTable(int32 id)
{
	FString rowName = FString::FromInt(id);
	auto row = _gunTable->FindRow<FGunData>(*rowName, TEXT(""));
	return *row;
}

TSubclassOf<AGunBase> UCGameInstance::GetGunClassFromTable(int32 id)
{
	FString rowName = FString::FromInt(id);
	auto row = _gunTable->FindRow<FGunData>(*rowName, TEXT(""));
	return row->_gunClass;
}

UTexture2D* UCGameInstance::GetGunPreviewFromTable(int32 id)
{
	FString rowName = FString::FromInt(id);
	auto row = _gunTable->FindRow<FGunData>(*rowName, TEXT(""));
	return row->_previewImage;
}

FUnitData UCGameInstance::GetUnitDataFromTable(int32 id)
{
	FString rowName = FString::FromInt(id);
	auto row = _unitTable->FindRow<FUnitData>(*rowName, TEXT(""));
	return *row;
}

TSubclassOf<class ACharacterBase> UCGameInstance::GetUnitClassFromTable(int32 id)
{
	FString rowName = FString::FromInt(id);
	auto row = _unitTable->FindRow<FUnitData>(*rowName, TEXT(""));
	return row->_unitClass;
}

FUnitPartStatArrayWrapper UCGameInstance::GetPartDataFromTable(FName partID)
{
	// PartDefinitionTable이 유효하지 않거나, PartID가 "None"이면
	if (!IsValid(_unitPartDefinitionTable) || partID.IsNone())
	{
		// 비어있는(기본값) FPartDefinitionRow를 반환합니다.
		return FUnitPartStatArrayWrapper();
	}

	// PartDefinitionTable에서 PartID로 Row를 찾습니다.
	FPartDefinitionRow* PartRow = _unitPartDefinitionTable->FindRow<FPartDefinitionRow>(partID, TEXT("GetPartDataFromTable Context"));

	if (PartRow)
	{
		// 찾았다면 해당 Row의 복사본을 반환
		FUnitPartStatArrayWrapper result;
		for (auto row : PartRow->Layers)
		{
			FUnitPartStat temp(row);
			result.PartStats.Add(temp);
		}

		return result;
	}
	else
	{
		// 테이블에 해당 ID가 없다면 경고 로그를 남기고 비어있는 Row 반환
		UE_LOG(LogTemp, Warning, TEXT("GetPartDataFromTable: Failed to find PartID '%s' in PartDefinitionTable."), *partID.ToString());
		return FUnitPartStatArrayWrapper();
	}
}

FStratagemSlot UCGameInstance::GetStratagemSlotFromTable(int32 id)
{
	FString rowName = FString::FromInt(id);
	auto row = _stratagemTable->FindRow<FStratagemSlot>(*rowName, TEXT(""));
	return *row;
}

TSubclassOf<class AStratagem> UCGameInstance::GetStratagemClassFromTable(int32 id)
{
	FString rowName = FString::FromInt(id);
	auto row = _stratagemTable->FindRow<FStratagemSlot>(*rowName, TEXT(""));
	return *row->StratagemClass;
}

void UCGameInstance::AddEarnedSample(const FSampleBundle& earnedSample)
{
	_savedSample.AddSample(earnedSample);
}

void UCGameInstance::InitializeUnitData()
{
	// 맵 비우기
	ProcessedUnitDataMap.Empty();

	// 데이터 테이블 에셋이 유효한지 확인 (에디터에서 할당되었는지)
	if (!IsValid(_unitTable) || !IsValid(_unitPartDefinitionTable))
	{
		UE_LOG(LogTemp, Error, TEXT("UMyGameInstance::InitializeUnitData: UnitDataTable or PartDefinitionTable is not valid!"));
		return;
	}

	// UnitDataTable의 모든 Row Name (Unit ID)을 가져옵니다.
	TArray<FName> UnitRowNames = _unitTable->GetRowNames();

	// 각 유닛 ID에 대해 루프를 돕니다.
	for (const FName& UnitID : UnitRowNames)
	{
		// FUnitData 테이블에서 해당 ID의 Row를 찾습니다.
		FUnitData* UnitDataRow = _unitTable->FindRow<FUnitData>(UnitID, TEXT("InitializeUnitData Context"));
		if (UnitDataRow == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to find UnitDataRow for ID: %s"), *UnitID.ToString());
			continue;
		}

		// 맵에 저장할 FProcessedUnitData 인스턴스 생성
		FProcessedUnitData ProcessedData;

		// 1. FUnitData의 기본 정보들을 ProcessedData로 복사
		ProcessedData._name = UnitDataRow->_name;
		ProcessedData._defaultMovementSpeed = UnitDataRow->_defaultMovementSpeed;
		ProcessedData._battleMovementSpeed = UnitDataRow->_battleMovementSpeed;
		ProcessedData._resistData = UnitDataRow->_resistData; // 구조체 통째로 복사
		ProcessedData._unitClass = UnitDataRow->_unitClass;
		ProcessedData._desc = UnitDataRow->_desc;

		// 2. FUnitData의 PartID들을 사용하여 FPartDefinitionRow 데이터를 찾아 ProcessedData에 할당
		// (GetPartDataFromTable 헬퍼 함수 사용)
		for (auto temp :UnitDataRow->_PartIDs)
		{
			
			ProcessedData._partDatas[temp.Key] = GetPartDataFromTable(*(temp.Value).ToString());
		}
		

		// 3. 완성된 ProcessedData를 맵에 추가
		ProcessedUnitDataMap.Add(UnitID, ProcessedData);
	}

	UE_LOG(LogTemp, Log, TEXT("UMyGameInstance: Successfully initialized %d units into ProcessedUnitDataMap."), ProcessedUnitDataMap.Num());
}

bool UCGameInstance::GetProcessedUnitData(FName UnitID, FProcessedUnitData& OutData)
{
	// 맵에서 UnitID(Key)에 해당하는 FProcessedUnitData(Value)를 찾습니다.
	const FProcessedUnitData* FoundData = ProcessedUnitDataMap.Find(UnitID);

	if (FoundData)
	{
		// 데이터를 찾았다면, 출력 변수(OutData)에 복사하고 true 반환
		OutData = *FoundData;
		return true;
	}

	// 데이터를 못 찾았다면, OutData는 변경하지 않고 false 반환
	UE_LOG(LogTemp, Warning, TEXT("GetProcessedUnitData: Failed to find processed data for UnitID: %s"), *UnitID.ToString());
	return false;
}
