// Fill out your copyright notice in the Description page of Project Settings.


#include "CGameInstance.h"

#include "Kismet/GameplayStatics.h"

#include "Game/PreDeployment/PreDeploymentState.h"
#include "Gun/GunBase.h"
#include "Data/GunDataTable.h"
#include "Data/OperationDataAsset.h"
#include "Data/MissionDataAsset.h"
#include "Data/ObjectiveDataAsset.h"

#include "StratagemComponent.h"
#include "Object/Stratagem/Stratagem.h"
#include "Object/AbnormalityTable.h"
#include "Object/CDamageType.h"

#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"


#include "Character/CharacterBase.h"
#include "Character/StatComponent.h"
#include "Character/UnitDataTable.h"

void UCGameInstance::Init()
{
	Super::Init();

	_preDeployState = NewObject<UPreDeploymentState>(this);

	CachingUnitDataFromTable();
	CachingAbnormalityDataFromTable();
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
void UCGameInstance::CachingUnitDataFromTable()
{
	// 1. 테이블 유효성 검사
	if (!_unitTable || !_unitPartDefinitionTable)
	{
		UE_LOG(LogTemp, Error, TEXT("CachingUnitDataFromTable failed: One or both DataTables are null."));
		return;
	}

	// 기존 맵 클리어 (재호출 대비)
	_processedUnitDataMap.Empty();

	// 2. _unitTable 순회 시작
	_unitTable->ForeachRow<FUnitData>(
		TEXT("UCGameInstance::CachingUnitDataFromTable"), 
		[&](const FName& Key, const FUnitData& Value)
		{
			FProcessedUnitData temp;

			// 기본 데이터 복사
			temp._battleMovementSpeed = Value._battleMovementSpeed;
			temp._defaultMovementSpeed = Value._defaultMovementSpeed;
			temp._name = Value._name; // FText
			temp._desc = Value._desc; // FText
			temp._resistData = Value._resistData;
			temp._unitClass = Value._unitClass;

			// 3. 부위 정의 테이블 순회 및 캐싱
			// Value._PartIDs의 Key는 EBodyPart, Value는 FName(PartDefinition Row Key)이라고 가정
			for (const auto& partID : Value._PartIDs)
			{
				const FName PartDefRowKey = partID.Value; // FName 키를 바로 사용 (효율적)

				// 3-1. 중첩된 테이블에서 행 검색 (NULL 체크 필수)
				const FPartDefinitionRow* row = _unitPartDefinitionTable->FindRow<FPartDefinitionRow>(PartDefRowKey, TEXT("PartDefLookup"));

				if (row)
				{
					// FProcessedUnitData에 부위 컨테이너 추가 (PartID.Key는 EBodyPart 타입)
					temp._partDatas.Add(partID.Key);

					// 3-2. 레이어 데이터 순회 및 Stat 구조체 생성
					for (const auto& part : row->Layers)
					{
						FUnitPartStat partStat;
						partStat.LayerName = part.LayerName;
						partStat._curHP = part._partHP;
						partStat._partHP = part._partHP;
						partStat._partAV = part._partAV;
						partStat._partDurability = part._partDurability;
						partStat._partExplosionImmunity = part._partExplosionImmunity;
						partStat._partInfluence = part._partInfluence;

						// FProcessedUnitData의 맵에 Stat 추가
						// temp._partDatas[EBodyPart] 컨테이너 내부에 PartStat을 추가
						temp._partDatas[partID.Key].PartStats.Add(partStat);
					}
				}
				else
				{
					// 디버깅을 위해 어느 유닛의 어떤 부위 정의를 찾지 못했는지 로그를 남김
					UE_LOG(LogTemp, Error, TEXT("Part Definition Error for Unit '%s': Missing row for PartDefKey '%s'."),
						*Key.ToString(), *PartDefRowKey.ToString());
				}
			}

			// 4. 최종 결과 맵에 저장
			_processedUnitDataMap.Add(temp._unitClass, temp);
		}
	);

	UE_LOG(LogTemp, Display, TEXT("UnitData Init Num : %d "),_processedUnitDataMap.Num());
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

void UCGameInstance::CachingAbnormalityDataFromTable()
{

	if (!_abnormalityTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("DataTable is null."));
		return;
	}

	_abnormalityTable->ForeachRow<FAbnormalityData>(TEXT("UCGameInstance::CachingAbnormalityDataFromTable"), [&](const FName& Key, const FAbnormalityData& Value)
		{
			FProcessedAbnormalityDefinitionData data;
			FCDamageEvent event;
			// 기본 데이터 복사
			data._abnormalityType = Value._abnormalityType;
			data._name= Value._name;
			data._stateDruration = Value._stateDuration;
			data._stateWeight = Value._stateWeight;
			data._statusID = (int32)Value._statusID;
		
			event.BaseDamage = Value._normalDamage;
			event.DurabilityDamage = Value._durabilityDamage;
			event.IsExplosionDamage = false;
			event.PenetrationLevel = Value._penetrationLevel;
			event.DemolitionDamage = 0;
			event.DamageTypeClass = Value._damageType;
			

			// 4. 최종 결과 맵에 저장
			_abnormalityDefinitions.Add(Value._statusID, data);
			_abnormalityDamageEvents.Add(Value._statusID, event);
		}
	);

	UE_LOG(LogTemp, Display, TEXT("AbnormalData Init Num :%d "), _abnormalityDefinitions.Num());
}

FProcessedAbnormalityDefinitionData UCGameInstance::GetAbnormalDefinitionDataFromTable(FName partID)
{
	return FProcessedAbnormalityDefinitionData();
}

const FProcessedAbnormalityDefinitionData* UCGameInstance::GetProcessedAbnormalityDefinitionData(EAbnormality state)
{
	return _abnormalityDefinitions.Find(state);
}

const FCDamageEvent* UCGameInstance::GetAbnormalDamageEventData(EAbnormality state)
{

	return _abnormalityDamageEvents.Find(state);
		
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

void UCGameInstance::AddRewardCurrency(const FPlayerCurrency& reward)
{
	_playerCurrency.AddCurrency(reward);
}

FSampleBundle UCGameInstance::GetSavedSample()
{
	if (!_playerCurrency._samples.IsEmpty())
		return _playerCurrency._samples;

	return FSampleBundle();
}

void UCGameInstance::InitializeUnitData()
{
	// 맵 비우기
	_processedUnitDataMap.Empty();

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
		_processedUnitDataMap.Add(ProcessedData._unitClass, ProcessedData);
	}

	UE_LOG(LogTemp, Log, TEXT("UCGameInstance: Successfully initialized %d units into ProcessedUnitDataMap."), _processedUnitDataMap.Num());
}

const FProcessedUnitData* UCGameInstance::GetProcessedUnitData(TSubclassOf<class ACharacterBase> UnitID)
{
	// 맵에서 UnitID(Key)에 해당하는 FProcessedUnitData(Value)를 찾습니다.
	const FProcessedUnitData* FoundData = _processedUnitDataMap.Find(UnitID);

	return FoundData;
}
