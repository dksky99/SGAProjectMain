// Fill out your copyright notice in the Description page of Project Settings.


#include "HellDiverAnimInstance.h"
#include "HellDiver.h"
#include "../../Gun/GunBase.h"
#include "HellDiverStateComponent.h"
#include "HellDiverStatComponent.h"
#include "HellDiverInvenComponent.h"
#include "Components/SkeletalMeshComponent.h"
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
	_hellDiver = Cast<AHellDiver>(TryGetPawnOwner());
	const FTransform suppliedWeaponOffset = _weaponOffset;
	const bool suppliedUseAim = _bUseAim;
	ResetRuntimeSnapshot();
	if (IsValid(_hellDiver))
	{
		// Do not overwrite an offset already supplied for this animation instance.
		_weaponOffset = suppliedWeaponOffset;
		_bUseAim = suppliedUseAim;
	}
	_refreshWeaponOffsetOnNextUpdate = !_bUseAim && !_weaponOffsetExplicitlyDisabled;
	UpdateLeftHandIKSnapshot();
}

void UHellDiverAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// A preview instance, a replaced pawn, or a destroyed owner must not retain a weapon.
	_hellDiver = Cast<AHellDiver>(TryGetPawnOwner());
	UHellDiverStateComponent* state = IsValid(_hellDiver) ? _hellDiver->GetStateComponent() : nullptr;
	UHellDiverInvenComponent* inventory = IsValid(_hellDiver) ? _hellDiver->GetInvenComponent() : nullptr;
	if (!IsValid(state) || !IsValid(inventory))
	{
		ResetRuntimeSnapshot();
		UpdateLeftHandIKSnapshot();
		return;
	}

	_characterState = state->GetCharacterState();
	_actionState = state->GetActionState();
	_weaponState = state->GetWeaponState();
	_lifeState = state->GetLifeState();
	_isFiring = state->IsFiring();
	_isReloading = state->IsReloading();
	_isRolling = state->IsRolling();
	_isFocusing = state->IsFocusing();
	_isAiming = state->IsAiming();

	AGunBase* equippedGun = inventory->GetEquippedGun();
	_curWeapon = IsValid(equippedGun) ? equippedGun : nullptr;
	USkeletalMeshComponent* gunMesh = IsValid(_curWeapon) ? _curWeapon->GetMesh() : nullptr;
	if (_refreshWeaponOffsetOnNextUpdate)
	{
		_refreshWeaponOffsetOnNextUpdate = false;
		USkeletalMeshComponent* characterMesh = _hellDiver->GetMesh();
		// Reinitialization may happen while the weapon is already held. Refresh once,
		// only after checking the same attachment contract used by AttachToHand.
		if (!_bUseAim && !_weaponOffsetExplicitlyDisabled
			&& _weaponState == EWeaponType::Gun && !state->IsWeaponChanging()
			&& IsValid(gunMesh) && IsValid(characterMesh)
			&& _curWeapon->GetOwnerCharacter() == _hellDiver
			&& gunMesh->GetAttachParent() == characterMesh
			&& gunMesh->GetAttachSocketName() == FName(TEXT("cc_weaponbone_r_socket"))
			&& characterMesh->GetAnimInstance() == this)
		{
			_curWeapon->CalculateMuzzleToHand_R();
		}
	}
	_muzzleTrans = FTransform::Identity;
	_muzzleTrans_Relation = FTransform::Identity;
	_leftHandTrans = FTransform::Identity;
	_jointTargetLoc = FVector::ZeroVector;
	if (IsValid(gunMesh) && IsValid(_hellDiver->GetMesh()))
	{
		// Legacy fields retain their original contracts, including hand_r-local LeftGrip.
		_muzzleTrans = _hellDiver->GetMuzzleTransform();
		_muzzleTrans_Relation = _hellDiver->GetMuzzleTransform_Relative();
		_leftHandTrans = _hellDiver->GetLeftHandSocketTransform();
		_jointTargetLoc = _hellDiver->GetJointTargetLocation();
	}
	else
	{
		// An unequipped/destroyed weapon must not keep the previous muzzle offset active.
		_weaponOffset = FTransform::Identity;
		_bUseAim = false;
	}

	// GetAimOffset/GetCurrentMoveNode remain disabled legacy paths.
	CheckEquipChange(state->GetEquipIndex());
	IsUsingLeftHand();
	if (state->IsWeaponChanging() || state->IsMotionChanging())
	{
		_useLeftHand = false;
	}
	UpdateLeftHandIKSnapshot();
	IsUsingFocusing();
	AimFocus(DeltaSeconds);
	// Right-hand aiming is independent of the animated left-hand release curve.
	_aimRigAlpha = _weaponState == EWeaponType::Gun && _bUseAim && _isFocusing
		&& _lifeState == ELifeState::Alive && IsValid(gunMesh) && !state->IsWeaponChanging()
		? 1.f : 0.f;
    _upperBodyAimAlpha = _characterState != ECharacterState::Proning && !_isRolling && !_isActing
        && !state->IsMotionChanging() ? _aimRigAlpha : 0.f;
    _upperBodyLeftHandAlpha = 0.f;
    _upperBodyLeftGripInHand = FTransform::Identity;
    if (_upperBodyAimAlpha > 0.f && _hasValidLeftHandIKTarget
        && gunMesh->DoesSocketExist(TEXT("Muzzle")))
    {
        // Use weapon-local socket geometry, not the previous evaluated world-space hand.
        const FTransform GripInGun = gunMesh->GetSocketTransform(TEXT("IK_left"), RTS_Component);
        const FTransform MuzzleInGun = gunMesh->GetSocketTransform(TEXT("Muzzle"), RTS_Component);
        const FTransform GripInHand = GripInGun.GetRelativeTransform(MuzzleInGun) * _weaponOffset;
        if (!GripInHand.ContainsNaN())
        {
            _upperBodyLeftGripInHand = GripInHand;
            _upperBodyLeftHandAlpha = _leftHandIKAlpha;
            // Do not overwrite the same-frame solve with the legacy world-space snapshot.
            _leftHandIKAlpha = 0.f;
        }
    }
    const FTransform MeshWorld = _hellDiver->GetMesh()->GetComponentTransform();
    _aimBodyForwardMesh = MeshWorld.InverseTransformVectorNoScale(_hellDiver->GetActorForwardVector()).GetSafeNormal();
    _aimBodyUpMesh = MeshWorld.InverseTransformVectorNoScale(_hellDiver->GetActorUpVector()).GetSafeNormal();
	DrawDebugs();
}

void UHellDiverAnimInstance::ResetRuntimeSnapshot()
{
	_aimRigAlpha = 0.f;
    _upperBodyAimAlpha = 0.f;
    _upperBodyLeftHandAlpha = 0.f;
    _upperBodyLeftGripInHand = FTransform::Identity;
    _aimBodyForwardMesh = FVector(0,1,0);
    _aimBodyUpMesh = FVector::UpVector;
	_refreshWeaponOffsetOnNextUpdate = false;
	_curWeapon = nullptr;
	_characterState = ECharacterState::Standing;
	_actionState = EActionState::None;
	_weaponState = EWeaponType::None;
	_lifeState = ELifeState::Alive;
	_isFiring = false;
	_isReloading = false;
	_isRolling = false;
	_isFocusing = false;
	_isAiming = false;
	_useLeftHand = false;
	_useLeftHandIk = false;
	_useFocusing = false;
	_changeWeapon = false;
	_curEquipIndex = 0;
	_focusRotate = FRotator::ZeroRotator;
	_focusAlpha = 0.f;
	_weaponOffset = FTransform::Identity;
	_bUseAim = false;
	_leftHandTrans = FTransform::Identity;
	_muzzleTrans = FTransform::Identity;
	_muzzleTrans_Relation = FTransform::Identity;
	_jointTargetLoc = FVector::ZeroVector;
	_currentMoveState.Empty();
	_currentLookState.Empty();
}

void UHellDiverAnimInstance::UpdateLeftHandIKSnapshot()
{
	// Called only from NativeInitialize/NativeUpdateAnimation on the game thread.
	_leftHandIKTargetWorld = FVector::ZeroVector;
	_leftHandIKAlpha = 0.f;
	_hasValidLeftHandIKTarget = false;

	USkeletalMeshComponent* characterMesh = GetSkelMeshComponent();
	if (IsValid(characterMesh))
	{
		const FVector fallbackWorld = characterMesh->DoesSocketExist(TEXT("hand_l"))
			? characterMesh->GetSocketLocation(TEXT("hand_l"))
			: characterMesh->GetComponentLocation();
		if (!fallbackWorld.ContainsNaN())
		{
			_leftHandIKTargetWorld = fallbackWorld;
		}
	}

	USkeletalMeshComponent* gunMesh = IsValid(_curWeapon) ? _curWeapon->GetMesh() : nullptr;
	if (IsValid(_hellDiver) && IsValid(gunMesh) && gunMesh->DoesSocketExist(TEXT("IK_left")))
	{
		const FVector targetWorld = gunMesh->GetSocketLocation(TEXT("IK_left"));
		if (!targetWorld.ContainsNaN())
		{
			_leftHandIKTargetWorld = targetWorld;
			_hasValidLeftHandIKTarget = true;
			// Preserve the existing animated release/reacquisition curve.
			const float disableLeftHandIK = GetCurveValue(TEXT("DisableLeftHandIK"));
			const float curveAlpha = FMath::IsFinite(disableLeftHandIK)
				? FMath::Clamp(1.f - disableLeftHandIK, 0.f, 1.f) : 0.f;
			_leftHandIKAlpha = _useLeftHand ? curveAlpha : 0.f;
		}
	}

	// Keep the eligibility flag separate from the left-hand release weight.
}

void UHellDiverAnimInstance::GetCurrentMoveNode()
{
	const int32 moveIndex = GetStateMachineIndex(TEXT("Move"));
	const int32 lookIndex = GetStateMachineIndex(TEXT("Look"));
	FString temp1 = moveIndex != INDEX_NONE ? GetCurrentStateName(moveIndex).ToString() : FString();
	FString temp2 = lookIndex != INDEX_NONE ? GetCurrentStateName(lookIndex).ToString() : FString();
	MoveStateChanged(temp1);
	LookStateChanged(temp2);
}

void UHellDiverAnimInstance::AnimNotify_Reload()
{
	UE_LOG(LogTemp, Log, TEXT("ReloadNotify"));
	if (IsValid(_hellDiver))
	{
		if (AGunBase* gun = _hellDiver->GetEquippedGun(); IsValid(gun))
		{
			gun->OnReloadSectionEnded();
		}
	}
}

void UHellDiverAnimInstance::AnimNotify_FootStep()
{
	
	if (!IsValid(_hellDiver) || !IsValid(_hellDiver->GetStatComponent())) return;
	const float defaultSpeed = _hellDiver->GetStatComponent()->GetDefaultSpeed();
	if (defaultSpeed <= SMALL_NUMBER) return;
	_hellDiver->MakeSound(_speed / defaultSpeed, "Footstep");

}

void UHellDiverAnimInstance::SetWeaponOffset(FTransform offset,bool useAim)
{
	// An explicit holster/disable event must win over deferred initialization.
	_refreshWeaponOffsetOnNextUpdate = false;
	_weaponOffsetExplicitlyDisabled = !useAim;
	_bUseAim = useAim;

	_weaponOffset = offset;


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
		//상태 변화가 생겼을떄 브로드캐스트
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
	if (!IsValid(gun))
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
	if (!IsValid(_hellDiver) || !IsValid(_hellDiver->GetStateComponent()))
		return false;
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

	FVector controlForward = (_targetPos_Mesh - aimTransform.GetLocation()).GetSafeNormal();
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

	FVector controlForward = (_targetPos_Mesh - aimTransform.GetLocation()).GetSafeNormal();
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
	FVector controlForward = (_targetPos_Mesh - gunTrans.GetLocation()).GetSafeNormal();

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
	FVector controlForward = (_targetPos_Mesh - gunTrans.GetLocation()).GetSafeNormal();

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

void UHellDiverAnimInstance::DrawDebugs()
{
	if (!IsValid(_pawn) || !IsValid(_hellDiver) || !_pawn->GetController() || !GetWorld())
		return;
	USkeletalMeshComponent* mesh = _hellDiver->GetMesh();
	if (!mesh) return;
	FTransform handTransform = mesh->GetSocketTransform(TEXT("hand_r"), RTS_World);
	FTransform muzzleTransform = _weaponOffset * handTransform;
	DrawDebugLine(GetWorld(), handTransform.GetLocation(), muzzleTransform.GetLocation(), FColor::Red);

	FVector muzzleEnd = muzzleTransform.GetLocation() + muzzleTransform.GetUnitAxis(EAxis::X) * 100.f;
	DrawDebugLine(GetWorld(),  muzzleTransform.GetLocation(), muzzleEnd, FColor::Green);


		DrawDebugSphere(
			GetWorld(),
			_targetPos_Mesh,          // 우리가 눈으로 확인할 그 문제의 타겟 좌표!
			15.0f,              // 구의 크기 (반지름)
			12,                 // 구의 디테일 (세그먼트 수)
			FColor::Red,        // 색상 (눈에 띄는 빨간색 추천)
			false,              // 영구 지속 여부 (매 프레임 업데이트 할 것이므로 false)
			-1.0f,              // 화면에 유지되는 시간 (-1.0f로 두면 한 프레임만 보이고 사라집니다)
			0,                  // Depth Priority (0이면 물체에 가려지고, 1이상이면 벽을 뚫고 보입니다)
			2.0f                // 선 두께
		);
	


}

bool UHellDiverAnimInstance::SetLeftHandLocation(FVector inHandLocation)
{
	return false;
}
