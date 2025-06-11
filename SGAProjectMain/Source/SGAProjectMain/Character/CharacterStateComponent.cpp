// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterStateComponent.h"

#include "GameFramework/Character.h"

// Sets default values for this component's properties
UCharacterStateComponent::UCharacterStateComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCharacterStateComponent::BeginPlay()
{
	Super::BeginPlay();

	_owner =Cast<ACharacter>( GetOwner());
	
}


// Called every frame
void UCharacterStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCharacterStateComponent::AddAbnormality(EAbnormalityState abnormality)
{

	_activeAbnormalities |= (int32)abnormality;

}

void UCharacterStateComponent::SubAbnormality(EAbnormalityState abnormality)
{
	_activeAbnormalities &= ~(int32)abnormality;
}

bool UCharacterStateComponent::CheckAbnormality(EAbnormalityState abnormality)
{

	return _activeAbnormalities & (int32)abnormality;

}

bool UCharacterStateComponent::IsMoving()
{
	if (_owner == nullptr)
		return false;
	return _owner->GetVelocity().Size2D() > 1.0f;;
}

