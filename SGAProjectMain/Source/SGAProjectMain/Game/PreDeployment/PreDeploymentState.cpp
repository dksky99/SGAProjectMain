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

	FName curOpID = saveGame->GetCurOperationID();
	if (curOpID.IsNone()) return;
	
	auto GI = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
	if (!GI) return;

	auto opData = GI->GetOperationDataAsset(curOpID);
	SetCurOperation(opData);

	for (auto& pair : _missions)
	{
		UMissionDataAsset* mission = pair.Key;
		if (saveGame->GetCompletedMissionIDs().Contains(mission->GetMissionID()))
			_missions[mission] = EMissionState::Cleared;
	}

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

	if (op)
	{
		_curMission = nullptr;
		_missions.Empty();

		auto missions = _curOperation->GetMissions();
		for (auto mission : missions)
		{
			_missions.Add(mission, EMissionState::Available);
		}
	}
	else
		_missions.Empty();
}

void UPreDeploymentState::SetCurMission(UMissionDataAsset* mission)
{
	_curMission = mission;

	bool hasMission = (_curMission != nullptr);

	if (_missionSelectedEvent.IsBound())
		_missionSelectedEvent.Broadcast(hasMission);
}

void UPreDeploymentState::ApplyMissionResult(UMissionDataAsset* mission, bool isCleared)
{
	if (!mission) return;
	if (!_missions.Contains(mission)) return;
	EMissionState state = isCleared ? EMissionState::Cleared : EMissionState::Failed;
	_missions[_curMission] = state;
	_curMission = nullptr;
}

void UPreDeploymentState::ResetOperation()
{
	_curOperation = nullptr;
	_curMission = nullptr;
	_missions.Empty();
}

bool UPreDeploymentState::IsOperationCleared()
{
	if (_missions.Num() == 0) return false;

	for (auto& pair : _missions)
	{
		if (pair.Value != EMissionState::Cleared)
			return false;
	}
	return true;
}

bool UPreDeploymentState::IsOperationFailed()
{
	for (auto& pair : _missions)
	{
		if (pair.Value == EMissionState::Failed)
			return true;
	}

	return false;
}

bool UPreDeploymentState::IsMissionCleared(UMissionDataAsset* mission)
{
	if (_missions.Contains(mission))
		return _missions[mission] == EMissionState::Cleared;
	
	return false;
}

int32 UPreDeploymentState::GetClearedMissionsNum()
{
	int32 clearedCount = 0;

	for (auto& pair : _missions)
	{
		if (pair.Value == EMissionState::Cleared)
			clearedCount++;
	}

	return clearedCount;
}
