// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyPatrolManager.h"
#include "../Character/Enemy/Squad/ReinforceSquadSpawner.h"
#include "../Character/Enemy/EnemySquad.h"
#include "../Character/Enemy/AI/Patrol/BT_Task_Patrol.h"
#include "../Character/Enemy/AI/Patrol/CPatrolPath.h"

// Sets default values
AEnemyPatrolManager::AEnemyPatrolManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//상태를 자주확인할 필요없다. 여유롭게 확인하자.
	PrimaryActorTick.TickInterval = 5.f;

}

// Called when the game starts or when spawned
void AEnemyPatrolManager::BeginPlay()
{
	Super::BeginPlay();
	Init();


}

void AEnemyPatrolManager::Init()
{

	FActorSpawnParameters param;
	param.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	for (auto pair : _squadList)
	{
		for (int i = 0; i < pair.Value; i++)
		{
			auto temp = GetWorld()->SpawnActor< AEnemySquad>(pair.Key, FVector::ZeroVector, FRotator::ZeroRotator, param);
			
			_squadPool.Add(temp);
		}
	}
	for (int i = 0; i < _squadPool.Num(); i++)
	{
		AReinforceSquadSpawner* spawner= GetWorld()->SpawnActor< AReinforceSquadSpawner>(AReinforceSquadSpawner::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, param);

		_spawners.Add(spawner);
	}

}

// Called every frame
void AEnemyPatrolManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GetExtraCallableSquad();
}


void AEnemyPatrolManager::GetExtraCallableSquad()
{
	for (int i = 0; i < _paths.Num(); i++)
	{
		auto squad = _squadPool.IsValidIndex(i) ? _squadPool[i]: nullptr;
		auto spawner = _spawners.IsValidIndex(i) ? _spawners[i]: nullptr;
		//패스에 맞는 스쿼드와 스포너 가 있는지 확인.
		if (!squad || !spawner)
			continue;

		//사용가능한 스쿼드
		if (CheckRestoredSquad(squad, spawner))
			DeploySquad(squad, spawner, _paths[i]);


	}




	return ;
}

bool AEnemyPatrolManager::DeploySquad(AEnemySquad* squad, class AReinforceSquadSpawner* spawner, ACPatrolPath* path)
{
	if (squad == nullptr)
		return false;
	if (path == nullptr)
		return false;
	if (squad->IsActivatedSquad() == true)
		return false;
	FVector targetLoc = path->GetStartPosition();

	squad->Command_Patrol(path);
	spawner->ActivateSpawner(squad, targetLoc);




	return true;
}

bool AEnemyPatrolManager::CheckRestoredSquad(AEnemySquad* squad, AReinforceSquadSpawner* spawner)
{
	if (squad->IsCallableSquad() == false)
		return false;
	if(spawner->IsActivatedSpawner())
		return false;
	return true;
}


