// Fill out your copyright notice in the Description page of Project Settings.


#include "ReloadBackpack.h"

void AReloadBackpack::BeginPlay()
{
	Super::BeginPlay();

	_curSpare = _reloadBackpackData._maxSpare;
}

void AReloadBackpack::ConsumeSpare(int32 amount)
{
	_curSpare -= amount;
}
