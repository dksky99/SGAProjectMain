// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyPatrolManager.h"
#include "../Character/Enemy/Squad/PatrolSquad.h"
#include "../Character/Enemy/AI/Patrol/BT_Task_Patrol.h"
#include "../Character/Enemy/AI/Patrol/CPatrolPath.h"
// Sets default values
AEnemyPatrolManager::AEnemyPatrolManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

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
			auto temp = GetWorld()->SpawnActor< APatrolSquad>(pair.Key, FVector::ZeroVector, FRotator::ZeroRotator, param);
			_squadPool.Add(temp);
		}
	}


}

// Called every frame
void AEnemyPatrolManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


bool AEnemyPatrolManager::DeploySquad(APatrolSquad* squad, ACPatrolPath* path)
{
	if (squad == nullptr)
		return false;
	if (path == nullptr)
		return false;
	if (squad->IsActivatedSquad() == true)
		return false;
	FVector targetLoc = path->GetStartPosition();





	return false;
}

