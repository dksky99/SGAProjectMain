// Fill out your copyright notice in the Description page of Project Settings.


#include "StationedSquad.h"

AStationedSquad::AStationedSquad()
{
}

void AStationedSquad::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Display, TEXT("StartSpawn"));
	GetWorld()->GetTimerManager().SetTimer(_GenerateTimer, this, &AStationedSquad::CallRemainUnit, _generateCoolDown, false);


}

void AStationedSquad::CallRemainUnit()
{
	UE_LOG(LogTemp, Display, TEXT("TryAddUnit"));
	auto extra = CheckExtraUnit();
	if (extra)
	{
		UE_LOG(LogTemp, Display, TEXT("SpawnUnit"));
		SpawnUnit(extra);
	}

	GetWorld()->GetTimerManager().SetTimer(_GenerateTimer,this, &AStationedSquad::CallRemainUnit, _generateCoolDown,false);


	return;
}

