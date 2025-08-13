// Fill out your copyright notice in the Description page of Project Settings.


#include "HellDiverAnimInstance.h"
#include "HellDiver.h"
#include "../../Gun/GunBase.h"
#include "HellDiverStateComponent.h"
#include "HellDiverStatComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimNode_StateMachine.h"
#include "Animation/AnimInstanceProxy.h"
#include "Perception/AISense_Hearing.h"

#include "Kismet/KismetMathLibrary.h"

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
			_leftHandTrans = _hellDiver->GetLeftHandSocketTransform();
			_jointTargetLoc = _hellDiver->GetJointTargetLocation();
			_isVaulting = _hellDiver->GetStateComponent()->IsVaulting();
			_targetPos = _hellDiver->GetTargetLoc();
			GetAimOffset();
			CheckEquipChange(_hellDiver->GetStateComponent()->GetEquipIndex());
			IsUsingLeftHand();
			IsUsingFocusing();
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

void UHellDiverAnimInstance::AnimNotify_Reload()
{
	UE_LOG(LogTemp, Log, TEXT("ReloadNotify"));
	if (AGunBase* gun = Cast<AGunBase>(_hellDiver->GetEquippedGun()))
	{
		gun->ChangeReloadStage();
	}
}

void UHellDiverAnimInstance::AnimNotify_FootStep()
{
	UE_LOG(LogTemp, Warning, TEXT("Footstep 애님 노티파이 실행됨"));
	
	if (!_hellDiver) return;

	UAISense_Hearing::ReportNoiseEvent(
		_hellDiver->GetWorld(),
		_hellDiver->GetActorLocation(),
		_moveSpeed/_hellDiver->GetStatComponent()->GetDefaultSpeed(),
		_hellDiver
	);


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
	
	if(_useFocusing)
	{
		FRotator temp = _hellDiver->Focusing();
		double dot = temp.Roll;
		temp.Roll = 0.0f;
		_focusRotate += temp * DeltaSeconds * _rotateSpeed * dot;
		_focusAlpha = FMath::Clamp(_focusAlpha + _focusSpeed * DeltaSeconds, 0, 1);
	}
	else
	{
		// 부드럽게 원래 자세로 돌아감
		_focusRotate = FMath::RInterpTo(_focusRotate, FRotator::ZeroRotator, DeltaSeconds, _rotateSpeed);

		_focusAlpha = FMath::Clamp(_focusAlpha - _focusSpeed * DeltaSeconds, 0, 1);

	}
}

bool UHellDiverAnimInstance::IsStableState_Move()
{

	if (_currentMoveState == "Sprinting")
		return true;
	if (_currentLookState == "Crouching")
		return true;
	if (_currentMoveState == "Standing")
		return true;
	if (_currentLookState == "Proning")
		return true;
	return false;
}

bool UHellDiverAnimInstance::IsStableState_Look()
{
	if(_currentLookState=="UnArmed")
		return true;
	if (_currentLookState == "Gun")
		return true;
	return false;
}

bool UHellDiverAnimInstance::IsUsingLeftHand()
{
	auto gun=_hellDiver->GetEquippedGun();
	_useLeftHand = false;

	if (_lifeState != ELifeState::Alive)
		return false;
	if (_characterState == ECharacterState::Knockdown)
		return false;
	if (_actionState != EActionState::None)
		return false;
	if (_isVaulting)
		return false;
	if (gun == nullptr)
		return false;
	if (_weaponState!=EWeaponType::Gun)
	{
		return false;
	}
	_useLeftHand = true;
	return true;
}

bool UHellDiverAnimInstance::IsUsingFocusing()
{
	_useFocusing = false;
	if (_lifeState != ELifeState::Alive)
		return false;
	if (_characterState == ECharacterState::Knockdown)
		return false;
	if (_actionState != EActionState::None)
		return false;
	if (_isVaulting)
		return false;
	if (_hellDiver->GetStateComponent()->IsWeaponChanging())
		return false;
	if (!_isFocusing && IsMoving())
		return false;
	if (IsStableState_Look() == false)
		return false;
	if (_isReloading)
		return false;
	_useFocusing = true;
	return true;
}

void UHellDiverAnimInstance::CheckEquipChange(uint8 index)
{
	if (_curEquipIndex == index)
	{
		_changeWeapon = false;
		return;
	}
	_changeWeapon = true;
	_curEquipIndex = index;

}

void UHellDiverAnimInstance::GetAimOffset()
{
	// 캐릭터 레퍼런스가 유효한지 확인하고, 없다면 가져옵니다.
	if (!_hellDiver)
	{
		auto pawn = TryGetPawnOwner();
		if (pawn)
		{
			_hellDiver = Cast<AHellDiver>(pawn);

		}
	}

	if (!_hellDiver)
	{
		return;
	}

	USkeletalMeshComponent* mesh = _hellDiver->GetMesh();
	if (!mesh)
	{
		return;
	}
	//정말 가장 확실한 메시의 로테이션을 가지고있는 루트를 가져와보자
	const FName rootBoneName = TEXT("root");
	 FTransform rootTransform = mesh->GetSocketTransform(rootBoneName);

	 //사격하는 위치와 높이와 가장 근접한 본의 로테이션을 가져와보자
	 const FName spineBoneName = TEXT("spine_3");
	 FTransform spineTransform = mesh->GetSocketTransform(spineBoneName);
	 //본의 위치와 타겟의 위치를 빼 조준할 방향을 조정한다.
	 FVector aimLine = _targetPos - spineTransform.GetLocation();
	 //루트는 발바닥 밑에 있으니 로테이션은 그대로 위치만 스파인 3번과 동일하게 한다.
	 rootTransform.SetLocation(spineTransform.GetLocation());
	 //
	 FTransform temp2 = rootTransform;
	 
	 
	const FRotator controlRotation = _hellDiver->GetControlRotation();
	const FRotator actorRotation = spineTransform.Rotator();
	// 2. 'spine_01' 본의 월드 회전 값

	 //기존의 오프셋과 
	 FVector spineUp = spineTransform.GetUnitAxis(EAxis::Z).GetSafeNormal(); // 캐릭터 상방
	 FRotator temp= UKismetMathLibrary::NormalizedDeltaRotator(actorRotation, controlRotation);
	
	// FQuat DeltaQuat = FQuat::FindBetweenNormals( FVector::UpVector, spineUp);
	//
	// // 필요하면 Rotator로 변환
	// FRotator DeltaRot = DeltaQuat.Rotator();
	// FRotator Result = UKismetMathLibrary::ComposeRotators(temp, DeltaRot);


	 _yaw = temp.Yaw;
	 _pitch = temp.Pitch;


}
