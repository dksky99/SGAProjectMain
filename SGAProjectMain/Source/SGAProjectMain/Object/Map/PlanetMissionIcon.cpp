// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetMissionIcon.h"

// Sets default values
APlanetMissionIcon::APlanetMissionIcon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = _mesh;

}

// Called when the game starts or when spawned
void APlanetMissionIcon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlanetMissionIcon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlanetMissionIcon::SetMissionData(UMissionDataAsset* mission)
{
	_missionData = mission;
}
