// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeploymentState.h"

void UPreDeploymentState::SetGunID(int32 id)
{
	if (id < 100)
		_primaryGunID = id;
	else if (id < 200)
		_secondaryGunID = id;
	else
		_supportGunID = id;
}
