// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetObjectiveIcon.h"

// Sets default values
APlanetObjectiveIcon::APlanetObjectiveIcon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = _mesh;

}

// Called when the game starts or when spawned
void APlanetObjectiveIcon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlanetObjectiveIcon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
