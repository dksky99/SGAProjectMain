// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeploymentState.h"
#include "../../CGameInstance.h"
#include "../../CSaveGame.h"
#include "../../Data/OperationDataAsset.h"
#include "../../Data/MissionDataAsset.h"

UPreDeploymentState::UPreDeploymentState()
{
	_stratagemIDs.Init(-1, 4);
}

void UPreDeploymentState::ApplySaveGameData(UCSaveGame* saveGame)
{
	ResetOperation();

	if (!saveGame) return;

	int32 operationIndex = saveGame->GetCurOperationIndex();
	if (operationIndex < 0) return;
	//FName curOpID = saveGame->GetCurOperationID();
	//if (curOpID.IsNone()) return;
	
	auto GI = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
	if (!GI) return;

	/*auto opData = GI->GetOperationDataAsset(curOpID);*/
	auto opData = GI->GetOperationDataAssetByIndex(operationIndex);
	SetCurOperation(opData);

	auto clearedMissionIndexes = saveGame->GetCompletedMissionIndexes();
	for (int32 index : clearedMissionIndexes)
	{
		if (_missionStates.IsValidIndex(index))
		{
			_missionStates[index] = EMissionState::Cleared;
		}
	}

	//for (auto& pair : _missions)
	//{
	//	UMissionDataAsset* mission = pair.Key;
	//	if (saveGame->GetCompletedMissionIDs().Contains(mission->GetMissionID()))
	//		_missions[mission] = EMissionState::Cleared;
	//}

	_curMission = nullptr;
}

void UPreDeploymentState::SetGunID(int32 id)
{
	if (id < 100)
		_primaryGunID = id;
	else if (id < 200)
		_secondaryGunID = id;
}

void UPreDeploymentState::SetStratagemID(int32 index, int32 id)
{
	if (index >= 0 && index < _stratagemIDs.Num())
	{
		_stratagemIDs[index] = id;
	}
}

void UPreDeploymentState::SetCurOperation(UOperationDataAsset* op)
{
	_curOperation = op;
	_missionStates.Empty();
	SetCurMission(-1, nullptr);

	if (op)
	{
		TArray<UMissionDataAsset*> missions = op->GetMissions();
		_missionStates.Init(EMissionState::Available, missions.Num());
		//_missions.Empty();

		//auto missions = _curOperation->GetMissions();
		//for (auto mission : missions)
		//{
		//	_missions.Add(mission, EMissionState::Available);
		//}
	}
}

void UPreDeploymentState::SetCurMission(int32 index, UMissionDataAsset* mission)
{
	if (!_curOperation)
	{
		_curMission = nullptr;
		_curMissionIndex = -1;
		if (_missionSelectedEvent.IsBound())
			_missionSelectedEvent.Broadcast(false);
		return;
	}

	const auto& missions = _curOperation->GetMissions();
	if (!missions.IsValidIndex(index) || missions[index] != mission)
	{
		// 잘못된 인덱스/포인터면 선택 해제
		_curMission = nullptr;
		_curMissionIndex = -1;
		if (_missionSelectedEvent.IsBound())
			_missionSelectedEvent.Broadcast(false);
		return;
	}

	_curMission = mission;
	_curMissionIndex = index;

	if (_missionSelectedEvent.IsBound())
		_missionSelectedEvent.Broadcast(true);
}

void UPreDeploymentState::ApplyMissionResult(UMissionDataAsset* mission, bool isCleared)
{
	if (!mission && !_curOperation)
		return;

	if (mission != _curMission)
		return;

	if (!_missionStates.IsValidIndex(_curMissionIndex))
		return;

	EMissionState state = isCleared ? EMissionState::Cleared : EMissionState::Failed;
	_missionStates[_curMissionIndex] = state;

	// 선택 해제
	_curMission = nullptr;
	_curMissionIndex = -1;

	if (_missionSelectedEvent.IsBound())
		_missionSelectedEvent.Broadcast(false);
}

void UPreDeploymentState::ResetOperation()
{
	_curOperation = nullptr;
	_curMission = nullptr;
	_missionStates.Empty();
}

bool UPreDeploymentState::IsOperationCleared()
{
	if (_missionStates.Num() == 0) return false;

	for (EMissionState state : _missionStates)
	{
		if (state != EMissionState::Cleared)
			return false;
	}

	return true;
}

bool UPreDeploymentState::IsOperationFailed()
{
	for (EMissionState state : _missionStates)
	{
		if (state == EMissionState::Failed)
			return true;
	}

	return false;
}

bool UPreDeploymentState::IsMissionCleared(int32 index)
{
	if (!_missionStates.IsValidIndex(index))
		return false;

	return _missionStates[index] == EMissionState::Cleared;
}

int32 UPreDeploymentState::GetClearedMissionsNum()
{
	int32 clearedCount = 0;

	for (EMissionState state : _missionStates)
	{
		if (state == EMissionState::Cleared)
			clearedCount++;
	}

	return clearedCount;
}
