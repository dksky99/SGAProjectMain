// Fill out your copyright notice in the Description page of Project Settings.


#include "ReloadBackpack.h"

void AReloadBackpack::BeginPlay()
{
	Super::BeginPlay();

	_curBullet = _reloadBackpackData._maxBullet;
}
