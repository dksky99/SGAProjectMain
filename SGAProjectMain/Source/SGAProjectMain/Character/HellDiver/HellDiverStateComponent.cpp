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

bool UHellDiverStateComponent::StartSprint()
{
	if (_characterState != ECharacterState::Standing)
		return false;

	_characterState = ECharacterState::Sprinting;
	

	return true;


}

bool UHellDiverStateComponent::FinishSprint()
{
	if (_characterState != ECharacterState::Sprinting)
		return false;
	_characterState = ECharacterState::Standing;
	return true;
}

bool UHellDiverStateComponent::StartCrouch()
{
	if (_characterState == ECharacterState::Crouching)
		return false;
	_characterState = ECharacterState::Crouching;
	return true;
}

bool UHellDiverStateComponent::FinishCrouch()
{
	if (_characterState != ECharacterState::Crouching)
		return false;
	_characterState = ECharacterState::Standing;
	return true;
}

bool UHellDiverStateComponent::StartProne()
{

	if (_characterState == ECharacterState::Proning)
		return false;
	_characterState = ECharacterState::Proning;
	return true;
}

bool UHellDiverStateComponent::FinishProne()
{
	if (_characterState != ECharacterState::Proning)
		return false;
	_characterState = ECharacterState::Standing;
	return true;
}

bool UHellDiverStateComponent::StartRolling()
{
	if (_characterState == ECharacterState::Proning||_isRolling)
		return false;

	_isRolling = true;
	return true;
}

bool UHellDiverStateComponent::FinishRolling()
{
	bool temp = _isRolling;
	_isRolling = false;
	return temp;

}

bool UHellDiverStateComponent::StartTPSAiming()
{

	return true;

}

bool UHellDiverStateComponent::FinishTPSAiming()
{
	return true;
}

