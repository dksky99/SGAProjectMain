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

void AStationedSquad::RecieveDamage(float damage, float armorPen)
{
	//방어력이 더 높으면 무시
	if (_defense > armorPen)
		return;
	_curDurability = FMath::Clamp(_curDurability - damage, 0, _curDurability);
	//내구가 다하면 붕괴.
	if (_curDurability <= 0)
		DestroyFactory();
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

	_curDurability = _maxDurability;

	UE_LOG(LogTemp, Display, TEXT("StartSpawn"));
	GetWorld()->GetTimerManager().SetTimer(_GenerateTimer, this, &AStationedSquad::CallRemainUnit, _generateCoolDown, false);
}

void AStationedSquad::DestroyFactory()
{
	Command_Deactivate();

}
