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
	if (TryMotionChange())
		return false;
	UE_LOG(LogTemp, Error, TEXT("StartSprint"));
	_characterState = ECharacterState::Sprinting;
	_waitingMove = "Sprinting";
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
	_isMotionChange = true;
	_characterState = ECharacterState::Standing;
	_waitingMove = "Standing";
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
	if (TryMotionChange())
		return false;
	UE_LOG(LogTemp, Error, TEXT("StartCrouch"));
	_characterState = ECharacterState::Crouching;
	_waitingMove = "Crouching";
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
	if (TryMotionChange())
		return false;
	UE_LOG(LogTemp, Error, TEXT("FinishCrouch"));
	_characterState = ECharacterState::Standing;
	_waitingMove = "Standing";
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
	if (TryMotionChange())
		return false;
	UE_LOG(LogTemp, Error, TEXT("StartProne"));
	_characterState = ECharacterState::Proning;
	_waitingMove = "Proning";
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
	if (TryMotionChange())
		return false;
	UE_LOG(LogTemp, Error, TEXT("FinishProne"));
	_characterState = ECharacterState::Standing;
	_waitingMove = "Standing";
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
	if (_isMotionChange)
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

void UHellDiverStateComponent::MoveChangeFinish(FString newState)
{
	UE_LOG(LogTemp, Error, TEXT("Try Move UnLock"));
	if (_isMotionChange == false)
		return;

	if (_waitingMove.IsEmpty())
		return;
	if (_waitingMove != newState)
	{
		UE_LOG(LogTemp, Error, TEXT("NotMatch : %s, %s"),*_waitingMove,*newState);

		return;
	}
	_waitingMove.Empty();
	_isMotionChange = false;
	UE_LOG(LogTemp, Error, TEXT("Move UnLock"));
}

void UHellDiverStateComponent::LookChangeFinish(FString newState)
{
	UE_LOG(LogTemp, Error, TEXT("Try Look UnLock"));
	if (_isWeaponChange == false)
		return;
	if (_waitingLook.IsEmpty())
		return;
	if (_waitingLook != newState)
	{

		UE_LOG(LogTemp, Error, TEXT("NotMatch : %s, %s"), *_waitingLook, *newState);
		return;
	}
	_waitingLook.Empty();
	_isWeaponChange = false;
	UE_LOG(LogTemp, Error, TEXT("Look UnLock"));

}

bool UHellDiverStateComponent::StartTPSAiming()
{

	return true;

}

bool UHellDiverStateComponent::FinishTPSAiming()
{
	return true;
}

bool UHellDiverStateComponent::TryMotionChange()
{
	if (_isMotionChange)
		return true;
	_isMotionChange = true;
	return false;
}

bool UHellDiverStateComponent::TryWeaponChange()
{
	if (_isWeaponChange)
		return true;
	_isWeaponChange = true;
	return false;
}

