// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySquad.h"
#include "Enemy.h"

// Sets default values
AEnemySquad::AEnemySquad()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemySquad::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemySquad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

