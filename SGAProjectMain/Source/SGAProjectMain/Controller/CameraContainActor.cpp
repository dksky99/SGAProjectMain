// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraContainActor.h"

#include "Camera/CameraComponent.h"
#include "Camera/CameraActor.h"
// Sets default values
ACameraContainActor::ACameraContainActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	SetRootComponent(_camera);
}

// Called when the game starts or when spawned
void ACameraContainActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACameraContainActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

