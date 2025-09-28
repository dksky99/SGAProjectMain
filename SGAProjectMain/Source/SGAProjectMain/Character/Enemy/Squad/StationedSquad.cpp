// Fill out your copyright notice in the Description page of Project Settings.


#include "StationedSquad.h"
#include "../Enemy.h"
#include "../../../Controller/EnemyController.h"
#include "NavigationSystem.h"
AStationedSquad::AStationedSquad()
{
}

void AStationedSquad::BeginPlay()
{
	Super::BeginPlay();
	Command_Stationed();
	_targetLoc = GetActorLocation();
	ActivateFactory();

}

void AStationedSquad::CallRemainUnit()
{
	UE_LOG(LogTemp, Display, TEXT("TryAddUnit"));
	if (_squadState == ESquadState::Deactivate)
		return;
	auto extra = CheckExtraUnit();
	if (extra)
	{
		UE_LOG(LogTemp, Display, TEXT("SpawnUnit"));
		SpawnUnit(extra);
		extra->Key->SetActorRotation(this->GetActorQuat());
		extra->Value->RecieveTargetLoc(MakeRandomLocation());
		
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
