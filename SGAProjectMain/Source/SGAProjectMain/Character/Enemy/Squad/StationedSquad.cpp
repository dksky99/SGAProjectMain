// Fill out your copyright notice in the Description page of Project Settings.


#include "StationedSquad.h"
#include "../Enemy.h"
AStationedSquad::AStationedSquad()
{
}

void AStationedSquad::BeginPlay()
{
	Super::BeginPlay();

	ActivateFactory();

}

void AStationedSquad::CallRemainUnit()
{
	UE_LOG(LogTemp, Display, TEXT("TryAddUnit"));

	auto extra = CheckExtraUnit();
	if (extra)
	{
		UE_LOG(LogTemp, Display, TEXT("SpawnUnit"));
		SpawnUnit(extra);
		extra->Key->SetActorRotation(this->GetActorQuat());
	}

	GetWorld()->GetTimerManager().SetTimer(_GenerateTimer,this, &AStationedSquad::CallRemainUnit, _generateCoolDown,false);


	return;
}

void AStationedSquad::ActivateFactory()
{

	UE_LOG(LogTemp, Display, TEXT("StartSpawn"));
	GetWorld()->GetTimerManager().SetTimer(_GenerateTimer, this, &AStationedSquad::CallRemainUnit, _generateCoolDown, false);
}

void AStationedSquad::DestroyFactory()
{
	Command_Deactivate();

}

