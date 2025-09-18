// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeploymentState.h"

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
	else
		_supportGunID = id;
}

void UPreDeploymentState::SetStratagemID(int32 index, int32 id)
{
	if (index >= 0 && index < _stratagemIDs.Num())
	{
		_stratagemIDs[index] = id;
	}
}
