// Fill out your copyright notice in the Description page of Project Settings.


#include "HellDiverStateComponent.h"

// Sets default values for this component's properties
UHellDiverStateComponent::UHellDiverStateComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHellDiverStateComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UHellDiverStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHellDiverStateComponent::StartSprint()
{
}

void UHellDiverStateComponent::FinishSprint()
{
}

void UHellDiverStateComponent::StartCroutch()
{
}

void UHellDiverStateComponent::FinishCroutch()
{
}

void UHellDiverStateComponent::StartProne()
{
}

void UHellDiverStateComponent::FinishProne()
{
}

