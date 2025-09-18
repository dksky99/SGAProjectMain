// Fill out your copyright notice in the Description page of Project Settings.


#include "CGameInstance.h"

#include "Kismet/GameplayStatics.h"

#include "Game/PreDeployment/PreDeploymentState.h"
#include "Gun/GunDataTable.h"
#include "Gun/GunBase.h"

#include "StratagemComponent.h"
#include "Object/Stratagem/Stratagem.h"

#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"


void UCGameInstance::Init()
{
	Super::Init();

	_preDeployState = NewObject<UPreDeploymentState>(this);
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

void UCGameInstance::AddEarnedSample(const FSampleBundle& earnedSample)
{
	_savedSample.AddSample(earnedSample);
}
