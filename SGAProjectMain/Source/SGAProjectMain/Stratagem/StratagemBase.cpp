// Fill out your copyright notice in the Description page of Project Settings.


#include "StratagemBase.h"

// Sets default values
AStratagemBase::AStratagemBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AStratagemBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AStratagemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

