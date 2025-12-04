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
			_curWeapon = _hellDiver->GetEquippedGun();
			_lifeState= _hellDiver->GetStateComponent()->GetLifeState();
			_isFiring= _hellDiver->GetStateComponent()->IsFiring();
			_isReloading= _hellDiver->GetStateComponent()->IsReloading();
			_isRolling= _hellDiver->GetStateComponent()->IsRolling();
			_isFocusing = _hellDiver->GetStateComponent()->IsFocusing();
			_isAiming= _hellDiver->GetStateComponent()->IsAiming();
			
			_muzzleTrans = _hellDiver->GetMuzzleTransform();
			_muzzleTrans_Relation = _hellDiver->GetMuzzleTransform_Relative();
			_leftHandTrans = _hellDiver->GetLeftHandSocketTransform();
			_jointTargetLoc = _hellDiver->GetJointTargetLocation();
			_targetPos = _hellDiver->GetTargetLoc();
			GetAimOffset(DeltaSeconds);
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
		gun->OnReloadSectionEnded();
	}
}

void UHellDiverAnimInstance::AnimNotify_FootStep()
{
	
	if (!_hellDiver) return;
	_hellDiver->MakeSound(_speed / _hellDiver->GetStatComponent()->GetDefaultSpeed(), "Footstep");

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
	auto gun=_curWeapon;
	_useLeftHand = false;

	if (_lifeState != ELifeState::Alive)
		return false;
	if (_characterState == ECharacterState::Knockdown)
		return false;
	if (_actionState != EActionState::None)
		return false;
	if (_isActing)
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
	if (_isActing)
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


//캐릭터의 메시가 돌아가게되는 60도마다의 turnleft나 turnright때 이전 로테이션에서의 에임오프셋때문에 조준이 튄다. 즉 에임오프셋에 메시의 로테이션이 관여해야한다.
//기존은 메시의 z와 y축을 기준으로 평면에 정면과 목표선을 투영해 각도를 판단했었다. 
//0. 이전 yaw pitch를 구함
//1. 우선 기본적인 에임오프셋을 구함.
//2. 총구의 forward와 목표선의 필요한 추가적인 yaw와 pitch를 구함.(중요한건 이 값은 시간에따라 계속 변동될것이라는것.
//3. 우선 이 둘을 합산하여 목표 값을 찾아본다. 그리고 이전 yaw pitch에서 lerp시킨다. 
//3. 이전 yaw와 이전 로테이션의 변화도 가지고있어보자. 그리고 이전로테이션과 지금의 로테이션의 변화를 가산해야할 값에 추가해놓는것이다 그러면 자연스러운 로테이션변화가 가능할것같다.
// 메시가 아닌 액터의 로테이션이다 
void UHellDiverAnimInstance::GetAimOffset(float deltaTime)
{
	float actorYaw = _hellDiver->GetActorRotation().Yaw;
	float yawChange = FMath::FindDeltaAngleDegrees( _prevActorYaw, actorYaw);
	_prevActorYaw = actorYaw;

	CalcYaw();
	CalcPitch();

	if (_weaponState != EWeaponType::Gun || _hellDiver->GetEquippedGun() == nullptr||_isActing||_isMoving&&!_isFocusing||!(_isFiring||_isFocusing))
	{
		_addYaw = 0.f;
		_addPitch = 0.f;
	}
	else
	{
		float prevAddYaw = _addYaw;
		float prevAddPitch = _addPitch;
		CalcAimYaw(deltaTime);
		CalcAimPitch(deltaTime);

		float interpSpeed = 50.f;

		_addYaw = FMath::FInterpTo(prevAddYaw, _addYaw, deltaTime,interpSpeed );
		_addPitch = FMath::FInterpTo(prevAddPitch, _addPitch, deltaTime, interpSpeed);
	}

	// 4. 최종 값 계산 (이전과 동일)
	_finalYaw = FMath::Clamp(_yaw + _addYaw - yawChange, -100.f, 100.f);
	_finalPitch = FMath::Clamp(_pitch + _addPitch, -130.f, 130.f);
}

void UHellDiverAnimInstance::CalcYaw()
{
	if (!_hellDiver)
	{
		auto pawn = TryGetPawnOwner();
		if (pawn) _hellDiver = Cast<AHellDiver>(pawn);
	}
	if (!_hellDiver) return;

	USkeletalMeshComponent* mesh = _hellDiver->GetMesh();
	if (!mesh) return;

	FTransform spineTransform = mesh->GetSocketTransform(TEXT("root"), RTS_World);
	FVector spineUp = spineTransform.GetUnitAxis(EAxis::Z).GetSafeNormal();
	FVector spineRight = spineTransform.GetUnitAxis(EAxis::X).GetSafeNormal();
	FVector spineFwd = spineTransform.GetUnitAxis(EAxis::Y).GetSafeNormal();

	FTransform aimTransform = mesh->GetSocketTransform(TEXT("spine_03"), RTS_World);
	FTransform temp = spineTransform;
	temp.SetLocation(aimTransform.GetLocation());

	FVector controlForward = (_targetPos - aimTransform.GetLocation()).GetSafeNormal();
	FVector charForward = spineFwd.GetSafeNormal();

	charForward = FVector::VectorPlaneProject(charForward, spineUp).GetSafeNormal();
	controlForward = FVector::VectorPlaneProject(controlForward, spineUp).GetSafeNormal();

	float dot = FVector::DotProduct(charForward, controlForward);
	float angleInDegree = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(dot, -1.0f, 1.0f)));
	FVector crossProduct = FVector::CrossProduct(charForward, controlForward);
	float directionSign = FVector::DotProduct(crossProduct, spineUp);
	float signedAngle = angleInDegree * FMath::Sign(directionSign);
	_yaw = signedAngle;
}

void UHellDiverAnimInstance::CalcPitch()
{
	if (!_hellDiver)
	{
		auto pawn = TryGetPawnOwner();
		if (pawn) _hellDiver = Cast<AHellDiver>(pawn);
	}
	if (!_hellDiver) return;

	USkeletalMeshComponent* mesh = _hellDiver->GetMesh();
	if (!mesh) return;

	FTransform spineTransform = mesh->GetSocketTransform(TEXT("root"), RTS_World);
	FVector spineUp = spineTransform.GetUnitAxis(EAxis::Z).GetSafeNormal();
	FVector spineRight = spineTransform.GetUnitAxis(EAxis::X).GetSafeNormal();
	FVector spineFwd = spineTransform.GetUnitAxis(EAxis::Y).GetSafeNormal();

	FTransform aimTransform = mesh->GetSocketTransform(TEXT("spine_03"), RTS_World);
	FTransform temp = spineTransform;
	temp.SetLocation(aimTransform.GetLocation());

	FVector controlForward = (_targetPos - aimTransform.GetLocation()).GetSafeNormal();
	FVector charForward = spineFwd.GetSafeNormal();

	charForward = FVector::VectorPlaneProject(charForward, spineRight).GetSafeNormal();
	controlForward = FVector::VectorPlaneProject(controlForward, spineRight).GetSafeNormal();

	float dot = FVector::DotProduct(charForward, controlForward);
	float angleInDegree = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(dot, -1.0f, 1.0f)));
	FVector crossProduct = FVector::CrossProduct(charForward, controlForward);
	float directionSign = FVector::DotProduct(crossProduct, spineRight);
	float signedAngle = angleInDegree * FMath::Sign(directionSign);
	_pitch = signedAngle;
}

void UHellDiverAnimInstance::CalcAimPitch(float deltaTime)
{
	if (!_hellDiver)
	{
		auto pawn = TryGetPawnOwner();
		if (pawn) _hellDiver = Cast<AHellDiver>(pawn);
	}
	if (!_hellDiver) return;

	USkeletalMeshComponent* mesh = _hellDiver->GetMesh();
	if (!mesh) return;

	FTransform gunTrans = _curWeapon->GetMuzzleTrans();
	FTransform spineTransform = mesh->GetSocketTransform(TEXT("root"), RTS_World);

	FVector spineUp = spineTransform.GetUnitAxis(EAxis::Z).GetSafeNormal();
	FVector spineRight = spineTransform.GetUnitAxis(EAxis::X).GetSafeNormal();

	FVector charForward = gunTrans.GetRotation().Vector().GetSafeNormal();
	FVector controlForward = (_targetPos - gunTrans.GetLocation()).GetSafeNormal();

	charForward = FVector::VectorPlaneProject(charForward, spineRight).GetSafeNormal();
	controlForward = FVector::VectorPlaneProject(controlForward, spineRight).GetSafeNormal();

	float dot = FVector::DotProduct(charForward, controlForward);
	if (dot > 0.9999f) return;

	float angleInDegree = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(dot, -1.0f, 1.0f)));
	FVector crossProduct = FVector::CrossProduct(charForward, controlForward);
	float directionSign = FVector::DotProduct(crossProduct, spineRight);
	float signedAngle =  FMath::Sign(directionSign);

	_addPitch += signedAngle * 100.f * (1.1 - dot) * (1.1 - dot);
	_addPitch = FMath::Clamp(_addPitch, -45.f, 45.f);
}

void UHellDiverAnimInstance::CalcAimYaw(float deltaTime)
{
	if (!_hellDiver)
	{
		auto pawn = TryGetPawnOwner();
		if (pawn) _hellDiver = Cast<AHellDiver>(pawn);
	}
	if (!_hellDiver) return;

	USkeletalMeshComponent* mesh = _hellDiver->GetMesh();
	if (!mesh) return;

	FTransform gunTrans = _curWeapon->GetMuzzleTrans();
	FTransform spineTransform = mesh->GetSocketTransform(TEXT("root"), RTS_World);

	FVector spineUp = spineTransform.GetUnitAxis(EAxis::Z).GetSafeNormal();
	FVector spineRight = spineTransform.GetUnitAxis(EAxis::X).GetSafeNormal();

	FVector charForward = gunTrans.GetRotation().Vector().GetSafeNormal();
	FVector controlForward = (_targetPos - gunTrans.GetLocation()).GetSafeNormal();

	charForward = FVector::VectorPlaneProject(charForward, spineUp).GetSafeNormal();
	controlForward = FVector::VectorPlaneProject(controlForward, spineUp).GetSafeNormal();

	float dot = FVector::DotProduct(charForward, controlForward);
	if (dot > 0.9999f) return;

	float angleInDegree = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(dot, -1.0f, 1.0f)));
	FVector crossProduct = FVector::CrossProduct(charForward, controlForward);
	float directionSign = FVector::DotProduct(crossProduct, spineUp);
	float signedAngle =  FMath::Sign(directionSign);

	_addYaw += signedAngle * 100.f * (1.1 - dot) * (1.1 - dot);
	_addYaw = FMath::Clamp(_addYaw, -45.f, 45.f);
}
