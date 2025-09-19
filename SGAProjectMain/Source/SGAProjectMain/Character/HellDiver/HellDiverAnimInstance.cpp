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
			
			_muzzleTrans = _hellDiver->GetMuzzleTransform();
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
	UE_LOG(LogTemp, Warning, TEXT("Footstep Notify"));
	
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

void UHellDiverAnimInstance::GetAimOffset(float deltaTime)
{


	CalcYaw();
	CalcPitch();
	if (_weaponState != EWeaponType::Gun || _hellDiver->GetEquippedGun() == nullptr)
	{
		_addYaw = 0.f;
		_addPitch = 0.f;
	}
	else
	{
		CalcAimYaw(deltaTime);
		CalcAimPitch(deltaTime);
	
	}
	_finalYaw = FMath::Clamp(_yaw + _addYaw,-90.f,90.f);
	_finalPitch = FMath::Clamp(_pitch + _addPitch, -90.f, 90.f);

}

void UHellDiverAnimInstance::CalcYaw()
{
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
	//메시의 방향을 명백히 해줄 척추의 윗쪽 축과 오른족 축을 기준으로 삼는다.
	FTransform spineTransform = mesh->GetSocketTransform(TEXT("root"), RTS_World);

	FVector spineUp = spineTransform.GetUnitAxis(EAxis::Z).GetSafeNormal(); // 캐릭터 상방
	FVector spineRight = spineTransform.GetUnitAxis(EAxis::X).GetSafeNormal();
	FVector spineFwd = spineTransform.GetUnitAxis(EAxis::Y).GetSafeNormal();


	//조준점과 가장 가깝고 영향이 큰 본. 
	FTransform aimTransform = mesh->GetSocketTransform(TEXT("spine_03"), RTS_World);

	
	FTransform temp = spineTransform;
	temp.SetLocation(aimTransform.GetLocation());

	//비교할 방향. 컨트롤러의 방향이나 조준선.
	FVector controlForward = _targetPos - aimTransform.GetLocation();
	//기준이 될 선.
	FVector charForward = spineFwd;
	controlForward = controlForward.GetSafeNormal();
	charForward = charForward.GetSafeNormal();

	// 두 선을 기준이되는 축을 법선으로하는 평면에 투영.
	charForward = FVector::VectorPlaneProject(charForward, spineUp).GetSafeNormal();
	controlForward = FVector::VectorPlaneProject(controlForward, spineUp).GetSafeNormal();

	//DrawDebugLine(GetWorld(), aimTransform.GetLocation(), aimTransform.GetLocation() + controlForward * 500.f, FColor::Red, false, 0.1f, 0, 2.0f);


	float dot = FVector::DotProduct(charForward, controlForward);
	float angleInRadians = FMath::Acos(FMath::Clamp(dot, -1.0f, 1.0f));
	float angleInDegree = FMath::RadiansToDegrees(angleInRadians);

	FVector crossProduct = FVector::CrossProduct(charForward, controlForward);

	// 외적 결과 벡터와 평면의 법선(spineUp)을 내적하여 방향을 확인합니다.
	float directionSign = FVector::DotProduct(crossProduct, spineUp);

	// --- 3. 최종 부호 있는 각도 계산 ---
	// DirectionSign이 양수이면 오른쪽(+), 음수이면 왼쪽(-)입니다.
	float signedAngle = angleInDegree * FMath::Sign(directionSign);


	_yaw = signedAngle;
	//UE_LOG(LogTemp, Display, TEXT("Yaw : %f "), _yaw);

}

void UHellDiverAnimInstance::CalcPitch()
{
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

	//메시의 방향을 명백히 해줄 척추의 윗쪽 축과 오른족 축을 기준으로 삼는다.
	FTransform spineTransform = mesh->GetSocketTransform(TEXT("root"), RTS_World);

	FVector spineUp = spineTransform.GetUnitAxis(EAxis::Z).GetSafeNormal(); // 캐릭터 상방
	FVector spineRight = spineTransform.GetUnitAxis(EAxis::X).GetSafeNormal();
	FVector spineFwd = spineTransform.GetUnitAxis(EAxis::Y).GetSafeNormal();

	//조준점과 가장 가깝고 영향이 큰 본. 
	FTransform aimTransform = mesh->GetSocketTransform(TEXT("spine_03"), RTS_World);
	
	FTransform temp = spineTransform;
	temp.SetLocation(aimTransform.GetLocation());
	//비교할 방향. 컨트롤러의 방향이나 조준선.
	FVector controlForward = _targetPos - aimTransform.GetLocation();
	//기준이 될 선.
	FVector charForward = spineFwd;
	controlForward = controlForward.GetSafeNormal();
	charForward = charForward.GetSafeNormal();
	// 두 선을 기준이되는 축을 법선으로하는 평면에 투영.
	charForward = FVector::VectorPlaneProject(charForward, spineRight).GetSafeNormal();
	controlForward = FVector::VectorPlaneProject(controlForward, spineRight).GetSafeNormal();



	//DrawDebugLine(GetWorld(), aimTransform.GetLocation(), aimTransform.GetLocation() + controlForward * 500.f, FColor::Yellow, false, 0.1f, 0, 2.0f);

	//두 선의 내적으로 일치하는정도를 확인.
	float dot = FVector::DotProduct(charForward, controlForward);
	//라디안으로 변환
	float angleInRadians = FMath::Acos(FMath::Clamp(dot, -1.0f, 1.0f));
	//각도로 변환
	float angleInDegree = FMath::RadiansToDegrees(angleInRadians);

	//둘을 외적.
	FVector crossProduct = FVector::CrossProduct(charForward, controlForward);

	// 외적 결과 벡터와 평면의 법선(spineUp)을 내적하여 방향을 확인합니다. 외적결과와의 내적이니 일치하거나 반대방향이거나 둘중하나가 나옴.

	float directionSign = FVector::DotProduct(crossProduct, spineRight);

	// --- 3. 최종 부호 있는 각도 계산 ---
	// DirectionSign이 양수이면 오른쪽(+), 음수이면 왼쪽(-)입니다.
	float signedAngle = angleInDegree * FMath::Sign(directionSign);

	_pitch = signedAngle;
	//UE_LOG(LogTemp, Display, TEXT("pitch : %f "), _pitch);


}




	//총구의 방향에 정확히 맞추는건 총구가 yaw와 pitch값에따라 유동적이고 회전의 주체가 총이 아니다보니 바로구하는방식은 쉽지않다
	// 맞을떄까지 값을 가중해가는 방법을 사용해보자
void UHellDiverAnimInstance::CalcAimPitch(float deltaTime)
{
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
	//총구의 트랜스폼
	FTransform gunTrans=_curWeapon->GetMuzzleTrans();


	//메시의 방향을 명백히 해줄 척추의 윗쪽 축과 오른족 축을 기준으로 삼는다. 지금은 루트의 축 또한 충분히 효과를 보여주기에 루트르 축으로둔다.
	FTransform spineTransform = mesh->GetSocketTransform(TEXT("root"), RTS_World);

	FVector spineUp = spineTransform.GetUnitAxis(EAxis::Z).GetSafeNormal(); // 캐릭터 상방
	FVector spineRight = spineTransform.GetUnitAxis(EAxis::X).GetSafeNormal();
	FVector spineFwd = spineTransform.GetUnitAxis(EAxis::Y).GetSafeNormal();



	//총구의 정면.
	FVector charForward = gunTrans.GetRotation().Vector().GetSafeNormal();
	// 조준해야할 방향.총구에서 목표의 위치를 향하게.
	FVector controlForward = (_targetPos - gunTrans.GetLocation()).GetSafeNormal();




	// 두 선을 기준이되는 축을 법선으로하는 평면에 투영.
	charForward = FVector::VectorPlaneProject(charForward, spineRight).GetSafeNormal();
	controlForward = FVector::VectorPlaneProject(controlForward, spineRight).GetSafeNormal();


	DrawDebugLine(GetWorld(), gunTrans.GetLocation(), gunTrans.GetLocation() + charForward * 500.f, FColor::Yellow, false, 0.1f, 0, 2.0f);
	DrawDebugLine(GetWorld(), gunTrans.GetLocation(), gunTrans.GetLocation() + controlForward * 500.f, FColor::Green, false, 0.1f, 0, 2.0f);


	//두 선의 내적으로 일치하는정도를 확인.1일수록 일치 -1이면 반대 0이면 수직
	float dot = FVector::DotProduct(charForward, controlForward);
	//UE_LOG(LogTemp, Display, TEXT("pitchDot : %f "), dot);
	//일치도가 일정이상 높아지면 굳이 값을 더할필요없다.
	if (dot > 0.99999f)
	{
		return;
	}
	//라디안으로 변환
	float angleInRadians = FMath::Acos(FMath::Clamp(dot, -1.0f, 1.0f));
	//Degree로 변환
	float angleInDegree = FMath::RadiansToDegrees(angleInRadians);

	//외적을 통해 어느 방향으로 차이가 나는지 확인 
	FVector crossProduct = FVector::CrossProduct(charForward, controlForward);

	// 외적 결과 벡터와 평면의 법선(spineUp)을 내적하여 방향을 확인합니다.
	float directionSign = FVector::DotProduct(crossProduct, spineRight);

	//UE_LOG(LogTemp, Display, TEXT("pitchDirection : %f "), directionSign);
	// --- 3. 최종 부호 있는 각도 계산 ---
	// DirectionSign이 양수이면 오른쪽(+), 음수이면 왼쪽(-)입니다.
	float signedAngle = 400.f * FMath::Sign(directionSign);


	_addPitch += signedAngle * deltaTime * (1.1f - dot);
	_addPitch = FMath::Clamp(_addPitch, -90.f, 90.f);
	//UE_LOG(LogTemp, Display, TEXT("AddPitch : %f "), _addPitch);
}

void UHellDiverAnimInstance::CalcAimYaw(float deltaTime)
{
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
	FTransform gunTrans = _curWeapon->GetMuzzleTrans();


	//메시의 방향을 명백히 해줄 척추의 윗쪽 축과 오른족 축을 기준으로 삼는다.
	FTransform spineTransform = mesh->GetSocketTransform(TEXT("root"), RTS_World);

	FVector spineUp = spineTransform.GetUnitAxis(EAxis::Z).GetSafeNormal(); // 캐릭터 상방
	FVector spineRight = spineTransform.GetUnitAxis(EAxis::X).GetSafeNormal();
	FVector spineFwd = spineTransform.GetUnitAxis(EAxis::Y).GetSafeNormal();



	//본의 정면 벡터를 가져옴.
	FVector charForward = gunTrans.GetRotation().Vector().GetSafeNormal();
	//비교할 방향. 컨트롤러의 방향이나 조준선.
	FVector controlForward = _targetPos - gunTrans.GetLocation();
	controlForward = controlForward.GetSafeNormal();
	charForward = charForward.GetSafeNormal();
	// 두 선을 기준이되는 축을 법선으로하는 평면에 투영.
	charForward = FVector::VectorPlaneProject(charForward, spineUp).GetSafeNormal();
	controlForward = FVector::VectorPlaneProject(controlForward, spineUp).GetSafeNormal();



	float dot = FVector::DotProduct(charForward, controlForward);

	if (dot > 0.99999f)
	{
		return;
	}
	float angleInRadians = FMath::Acos(FMath::Clamp(dot, -1.0f, 1.0f));
	float angleInDegree = FMath::RadiansToDegrees(angleInRadians);

	FVector crossProduct = FVector::CrossProduct(charForward, controlForward);

	// 외적 결과 벡터와 평면의 법선(spineUp)을 내적하여 방향을 확인합니다.
	float directionSign = FVector::DotProduct(crossProduct, spineUp);

	// --- 3. 최종 부호 있는 각도 계산 ---
	// DirectionSign이 양수이면 오른쪽(+), 음수이면 왼쪽(-)입니다.
	float signedAngle = 400.f * FMath::Sign(directionSign);


	_addYaw += signedAngle * deltaTime * (1.1f - dot);

	_addYaw = FMath::Clamp(_addYaw, -90.f, 90.f);
	//UE_LOG(LogTemp, Display, TEXT("AddYaw : %f "), _addYaw);


}
