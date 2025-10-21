// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeploymentState.h"
#include "../../Data/OperationDataAsset.h"

UPreDeploymentState::UPreDeploymentState()
{
	_stratagemIDs.Init(-1, 4);
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
	auto missions = _curOperation->GetMissions();
	for (auto mission : missions)
	{
		_missions.Add(mission, EMissionState::Available);
	}
}

void UPreDeploymentState::SetCurMission(UMissionDataAsset* mission)
{
	_curMission = mission;
	if (_missionSelectedEvent.IsBound())
		_missionSelectedEvent.Broadcast();
}

void UPreDeploymentState::ApplyMissionResult(bool isCleared)
{
	EMissionState state = isCleared ? EMissionState::Cleared : EMissionState::Failed;
	_missions[_curMission] = state;
	_curMission = nullptr;
}

void UPreDeploymentState::ClearOperation()
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
