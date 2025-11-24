// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyReinforceManager.h"
#include "../Character/Enemy/Enemy.h"
#include "../Character/Enemy/EnemySquad.h"
#include "../Character/Enemy/Squad/ReinforceSquadSpawner.h"
#include "EnemyPatrolManager.h"

// Sets default values
AEnemyReinforceManager::AEnemyReinforceManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemyReinforceManager::BeginPlay()
{
	Super::BeginPlay();
	Init();
	
}

void AEnemyReinforceManager::Init()
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

	for (int i = 0; i < 5; i++)
	{
		
		auto temp = GetWorld()->SpawnActor< AReinforceSquadSpawner>(AReinforceSquadSpawner::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, param);
		_spawnerPool.Add(temp);
	}

}

ASquadSpawner* AEnemyReinforceManager::GetExtraSpawner()
{
	for (auto squad : _spawnerPool)
	{
		if (squad->IsActivatedSpawner()==false)
			return squad;
	}

	return nullptr;
}

// Called every frame
void AEnemyReinforceManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

AEnemySquad* AEnemyReinforceManager::GetExtraCallableSquad(FVector callPoint,AActor* target)
{
	UE_LOG(LogTemp, Display, TEXT("CallReinforcement : %f %f %f"), callPoint.X, callPoint.Y, callPoint.Z);
	
	auto spawner = GetExtraSpawner();
	//여유가 되는 스포너가 있나
	if (spawner == nullptr)
		return nullptr;
	for (auto& squad : _squadPool)
	{
		FVector recivePoint;
		if(squad->IsCallableSquad())
		{
			UE_LOG(LogTemp, Display, TEXT("Call Success"));
			//호출하고 반환. 소환시도했는데 true면 이대로 끝 아니라면 continue로 다른 스쿼드 확인.
			SetSpawner(recivePoint, squad, spawner);
			
			return squad;

			continue;

		}
	}

	UE_LOG(LogTemp, Error, TEXT("Call Fail"));
	return nullptr;
}

void AEnemyReinforceManager::SetSpawner(FVector callPoint, AEnemySquad* squad, ASquadSpawner* spawner)
{
	//증원스포너는 일단 유닛이 있는 위치, 혹은 미션지 주변에 생성된다. 미션지의 핵심적인 오브젝트에 광범위한 네비 인보커를 달아놓을 예정이다.
	//그러니 네비가 깔려있는위치인지 확인하지 않겠다.
	spawner->ActivateSpawner(squad, callPoint);



}
