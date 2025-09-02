// Fill out your copyright notice in the Description page of Project Settings.


#include "CGameInstance.h"

#include "Game/PreDeployment/PreDeploymentState.h"
#include "Gun/GunDataTable.h"
#include "Gun/GunBase.h"

void UCGameInstance::Init()
{
	Super::Init();

	_preDeployState = NewObject<UPreDeploymentState>(this);
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
	return *row->_gunClass;
}

void UCGameInstance::AddEarnedSample(const FSampleBundle& earnedSample)
{
	_savedSample.AddSample(earnedSample);
}
