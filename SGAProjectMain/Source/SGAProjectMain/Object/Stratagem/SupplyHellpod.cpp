// Fill out your copyright notice in the Description page of Project Settings.


#include "SupplyHellpod.h"

// Sets default values
ASupplyHellpod::ASupplyHellpod()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	RootComponent = _mesh;

}

// Called when the game starts or when spawned
void ASupplyHellpod::BeginPlay()
{
	Super::BeginPlay();
	

}

// Called every frame
void ASupplyHellpod::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

