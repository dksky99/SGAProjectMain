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

#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"


void UCGameInstance::Init()
{
	Super::Init();

	_preDeployState = NewObject<UPreDeploymentState>(this);

	for (auto& operation : _allOperations)
	{
		_operationMap.Add(operation->GetOperationID(), operation);
	}
	for (auto& mission : _allMissions)
	{
		_missionMap.Add(mission->GetMissionID(), mission);
	}
	for (auto& objective : _allObjectives)
	{
		_objectiveMap.Add(objective->GetObjectiveID(), objective);
	}
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
