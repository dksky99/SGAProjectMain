// Fill out your copyright notice in the Description page of Project Settings.


#include "CGameInstance.h"

#include "Gun/GunDataTable.h"

FGunData UCGameInstance::GetGunDataFromTable(int32 id)
{
	FString rowName = FString::FromInt(id);
	auto row = _gunTable->FindRow<FGunData>(*rowName, TEXT(""));
	return *row;
}
