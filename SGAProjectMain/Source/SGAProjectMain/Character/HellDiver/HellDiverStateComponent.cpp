// Fill out your copyright notice in the Description page of Project Settings.


#include "HellDiverStateComponent.h"

// Sets default values for this component's properties
UHellDiverStateComponent::UHellDiverStateComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UHellDiverStateComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


bool UHellDiverStateComponent::StartSprint()
{

	if (!IsActionable())
		return false;
	if (_characterState != ECharacterState::Standing)
		return false;
	if (_isFiring)
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

	if (!IsActionable())
		return false;
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

	if (!IsActionable())
		return false;
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

	if (!IsActionable())
		return false;
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
	//행동불능상태에선 상태를 바꿀 수 없다.
	if (!IsActionable())
		return false;
	//누운상태에선 누울 수 없다.
	if (_characterState == ECharacterState::Proning)
		return false;
	//모션 변경을 시도.
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
	//일어나는건 누워있는 상태에서만 가능하다.
	if (_characterState != ECharacterState::Proning)
		return false;
	//롤링중엔 불가하다.
	if (_isRolling)
		return false;
	//행동을 변경한다.
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

	// 누워있는 상태가 아니어야하고 롤링중이 아니어야한다.
	if (_characterState == ECharacterState::Proning||_isRolling)
		return false;
	// 행동이 변경중엔 불가하다.
	if (_isMotionChange)
		return false;

	UE_LOG(LogTemp, Error, TEXT("StartRolling"));
	_isRolling = true;
	return true;
}

bool UHellDiverStateComponent::FinishRolling()
{

	_isRolling = false;
	UE_LOG(LogTemp, Error, TEXT("FinishRolling"));
	return true;

}

bool UHellDiverStateComponent::StartReload()
{
	UE_LOG(LogTemp, Error, TEXT("Call StartReload"));
	_isReloading = true;
	return true;
}

bool UHellDiverStateComponent::FinishReload()
{
	UE_LOG(LogTemp, Error, TEXT("Call ReloadFinish"));
	_isReloading = false;
	return true;
}

bool UHellDiverStateComponent::StartPakour()
{
	//지금 무언가 행동중이라면 파쿠르를 시전하지 못한다.
	if (_isActing)
		return false;
	//엎드리거나 몸을던지는중에선 불가하다.
	if (_characterState == ECharacterState::Proning || _isRolling)
		return false;

	_isActing = true;

	return true;
}

bool UHellDiverStateComponent::FinishPakour()
{
	if (_isActing == false)
		return false;

	_isActing = false;
	

	return true;
}

bool UHellDiverStateComponent::IsFocusing()
{
	//조준을 유지하는 상황.
	//행동불능상태가 아니여야한다.
	if (!IsActionable())
		return false;
	//None상태가 아니라면 false. 맵을 보거나 물체와 상호작용중이거나 스트라타젬을 입력중이거나.
	if (_actionState != EActionState::None)
		return false;
	//재장전중에도 
	if (_isReloading)
		return false;
	//무기를 변경중에도
	if (_isWeaponChange)
		return false;
	//행동중에도
	if (_isActing)
		return false;
	//사격중에는 무조껀 true
	if (_isFiring)
		return true;
	//tpsZoom상태나 fpv상태에선 무조껀
	if (IsAiming())
		return true;
	//평범한상태에서도 조준을 하지만 이동중에는 그방향으로 몸통을 옮기기 때문에 포커싱중이면 안된다.
	if (IsMoving())
		return false;

	return true;
}

void UHellDiverStateComponent::KnockDown()
{
	_characterState = ECharacterState::Knockdown;



}

void UHellDiverStateComponent::KnockDownRecovery()
{
	_characterState = ECharacterState::Proning;
}

void UHellDiverStateComponent::Dead()
{
	_lifeState = ELifeState::Dead;
}

bool UHellDiverStateComponent::IsActionable()
{
	//행동을 위해선 살아있어야하고 행동불능의 상태가 아니어야한다.
	if(_lifeState!=ELifeState::Alive)	
		return false;
	if (_characterState == ECharacterState::Knockdown)
		return false;
	if (IsUnable())
		return false;




	return true;
}

bool UHellDiverStateComponent::IsMovable()
{
	//행동 가능한 상태여야하고 몸을 던진상태가 아니여야한다.
	if (IsActionable() == false)
		return false;
	if (_isRolling)
		return false;

	return false;
}

void UHellDiverStateComponent::MoveChangeFinish(FString newState)
{
	//행동입력을 받고 바로 상태가 바뀌지 않도록 아직 일어나지않았는데 이동속도가 일어나있는 상태의 수준이 되선 안된다.
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
	//무기 교체가 안됬는데도 마찬가지.
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

//행동 변경중엔 다른 상태로 변경이 안됬으면 
bool UHellDiverStateComponent::TryMotionChange()
{
	if (_isMotionChange)
		return true;
	_isMotionChange = true;
	return false;
}
//무기 교체중엔 추가적인 무기교체 입력이 불가.
bool UHellDiverStateComponent::TryWeaponChange()
{
	if (_isWeaponChange)
		return true;
	_isWeaponChange = true;
	return false;
}

