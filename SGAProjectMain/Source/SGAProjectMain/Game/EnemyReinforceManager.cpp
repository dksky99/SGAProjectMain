// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyReinforceManager.h"
#include "../Character/Enemy/Enemy.h"
#include "../Character/Enemy/Squad/ReinforcementSquad.h"

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
			auto temp = GetWorld()->SpawnActor< AReinforcementSquad>(pair.Key, FVector::ZeroVector, FRotator::ZeroRotator, param);
			_squadPool.Add(temp);
		}
	}


}

// Called every frame
void AEnemyReinforceManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

AReinforcementSquad* AEnemyReinforceManager::GetExtraCallableSquad(FVector callPoint)
{
	UE_LOG(LogTemp, Display, TEXT("CallReinforcement : %f %f %f"), callPoint.X, callPoint.Y, callPoint.Z);
	for (auto& squad : _squadPool)
	{
		if(squad->CheckAbleToCall(callPoint,callPoint))
		{
			UE_LOG(LogTemp, Display, TEXT("Call Success"));
			return squad;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("Call Fail"));
	return nullptr;
}

