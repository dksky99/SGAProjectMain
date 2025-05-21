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

	if (_isRolling)
		return false;
	UE_LOG(LogTemp, Error, TEXT("StartSprint"));
	_characterState = ECharacterState::Sprinting;
	if (_characterStateChanged.IsBound())
	{

		_characterStateChanged.Broadcast(_characterState);
	}

	return true;


}

bool UHellDiverStateComponent::FinishSprint()
{
	if (_characterState != ECharacterState::Sprinting)
		return false;
	if (_isRolling)
		return false;
	UE_LOG(LogTemp, Error, TEXT("FinishSprint"));
	_characterState = ECharacterState::Standing;
	if (_characterStateChanged.IsBound())
	{

		_characterStateChanged.Broadcast(_characterState);
	}
	return true;
}

bool UHellDiverStateComponent::StartCrouch()
{
	if (_characterState == ECharacterState::Crouching)
		return false;
	if (_isRolling)
		return false;
	UE_LOG(LogTemp, Error, TEXT("StartCrouch"));
	_characterState = ECharacterState::Crouching;
	if (_characterStateChanged.IsBound())
	{

		_characterStateChanged.Broadcast(_characterState);
	}
	return true;
}

bool UHellDiverStateComponent::FinishCrouch()
{
	if (_characterState != ECharacterState::Crouching)
		return false;
	if (_isRolling)
		return false;
	UE_LOG(LogTemp, Error, TEXT("FinishCrouch"));
	_characterState = ECharacterState::Standing;
	if (_characterStateChanged.IsBound())
	{

		_characterStateChanged.Broadcast(_characterState);
	}
	return true;
}

bool UHellDiverStateComponent::StartProne()
{

	if (_characterState == ECharacterState::Proning)
		return false;
	UE_LOG(LogTemp, Error, TEXT("StartProne"));
	_characterState = ECharacterState::Proning;
	if (_characterStateChanged.IsBound())
	{

		_characterStateChanged.Broadcast(_characterState);
	}
	return true;
}

bool UHellDiverStateComponent::FinishProne()
{
	if (_characterState != ECharacterState::Proning)
		return false;
	if (_isRolling)
		return false;
	UE_LOG(LogTemp, Error, TEXT("FinishProne"));
	_characterState = ECharacterState::Standing;
	if (_characterStateChanged.IsBound())
	{

		_characterStateChanged.Broadcast(_characterState);
	}
	return true;
}

bool UHellDiverStateComponent::StartRolling()
{
	if (_characterState == ECharacterState::Proning||_isRolling)
		return false;

	UE_LOG(LogTemp, Error, TEXT("StartRolling"));
	_isRolling = true;
	return true;
}

bool UHellDiverStateComponent::FinishRolling()
{
	bool temp = _isRolling;
	_isRolling = false;
	UE_LOG(LogTemp, Error, TEXT("FinishRolling"));
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

