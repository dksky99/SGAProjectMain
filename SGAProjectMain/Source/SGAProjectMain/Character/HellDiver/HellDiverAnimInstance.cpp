// Fill out your copyright notice in the Description page of Project Settings.


#include "HellDiverAnimInstance.h"
#include "HellDiver.h"
#include "HellDiverStateComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimNode_StateMachine.h"
#include "Animation/AnimInstanceProxy.h"

UHellDiverAnimInstance::UHellDiverAnimInstance()
{
	
}

void UHellDiverAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	auto pawn = TryGetPawnOwner();
	if (pawn)
	{
		_hellDiver = Cast<AHellDiver>(pawn);

	}
}

void UHellDiverAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (_hellDiver)
	{
		if (_hellDiver != nullptr)
		{
			_characterState= _hellDiver->GetStateComponent()->GetCharacterState();
			_actionState= _hellDiver->GetStateComponent()->GetActionState();
			_weaponState= _hellDiver->GetStateComponent()->GetWeaponState();
			_lifeState= _hellDiver->GetStateComponent()->GetLifeState();
			_isFiring= _hellDiver->GetStateComponent()->IsFiring();
			_isReloading= _hellDiver->GetStateComponent()->IsReloading();
			_isRolling= _hellDiver->GetStateComponent()->IsRolling();
			_isFocusing = _hellDiver->GetStateComponent()->IsFocusing();
			AimFocus(DeltaSeconds);
			GetCurrentMoveNode();


		}
	}
}

void UHellDiverAnimInstance::GetCurrentMoveNode()
{
	FString temp1 = GetCurrentStateName(GetStateMachineIndex(TEXT("Move"))).ToString();
	FString temp2 = GetCurrentStateName(GetStateMachineIndex(TEXT("Look"))).ToString();
	MoveStateChanged(temp1);
	LookStateChanged(temp2);
}

bool UHellDiverAnimInstance::MoveStateChanged(FString curState)
{
	if (_currentMoveState==curState)
	{
		return false;
	}

	UE_LOG(LogTemp, Error, TEXT("Move : %s"), *curState);
	_currentMoveState = curState;
	if (_moveChanged.IsBound())
	{
		_moveChanged.Broadcast(curState);
	}
	return true;

}

bool UHellDiverAnimInstance::LookStateChanged(FString curState)
{
	if (_currentLookState == curState)
	{
		return false;
	}

	UE_LOG(LogTemp, Error, TEXT("Look : %s"), *curState);
	_currentLookState = curState;
	if (_lookChanged.IsBound())
	{
		_lookChanged.Broadcast(curState);
	}
	return true;
}

void UHellDiverAnimInstance::AimFocus(float DeltaSeconds)
{
	if (true )
	{
		FRotator temp = _hellDiver->Focusing();
		double dot = temp.Roll;
		temp.Roll = 0.0f;
		_focusRotate += temp * DeltaSeconds * _rotateSpeed * dot;
	}
	else
	{

		_focusRotate.Yaw /= 2.f;
		_focusRotate.Pitch /= 2.f;
	}
}
