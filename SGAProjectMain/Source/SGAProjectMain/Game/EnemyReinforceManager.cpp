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



}

// Called every frame
void AEnemyReinforceManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

