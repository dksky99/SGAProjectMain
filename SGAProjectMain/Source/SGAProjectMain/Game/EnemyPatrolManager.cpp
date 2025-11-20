// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyPatrolManager.h"

// Sets default values
AEnemyPatrolManager::AEnemyPatrolManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemyPatrolManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemyPatrolManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

