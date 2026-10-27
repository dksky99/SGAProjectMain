// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"

#include "CharacterBase.h"
#include "PlayerCharacter.h"
#include "CharacterStateComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Animation/AnimMontage.h"


#include "KismetAnimationLibrary.h"

UCharacterAnimInstance::UCharacterAnimInstance()
{
}

void UCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();


	TryGetCBOwner();

}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (_pawn == nullptr)
	{
		_pawn = Cast<ACharacterBase>(TryGetPawnOwner());

	}

	if (_pawn)
	{
		_speed = _pawn->GetVelocity().Size();
		_isFalling = _pawn->GetMovementComponent()->IsFalling();

		_deltaAngle = _pawn->MyDeltaAngle();
		_isActing = _pawn->GetStateComponent()->IsActing();

		FRotator controlRotation = _pawn->GetControlRotation();
		FRotator actorRotation = _pawn->GetActorRotation();

		_moveRotation = UKismetAnimationLibrary::CalculateDirection(_pawn->GetVelocity(), actorRotation);

		_isTurnLeft = _pawn->_isTurnLeft;
		_isTurnRight = _pawn->_isTurnRight;
			
	}

	if (_pawn)
	{
		_characterVelocity = _pawn->GetCharacterMovement()->Velocity;
		
		_acceleration = _pawn->GetCharacterMovement()->GetCurrentAcceleration();
		

		_lastFrameWorldLocation = _worldLocation;
		_worldLocation= _pawn->GetActorLocation();

		_worldRotation = _pawn->GetActorRotation();
		//타겟이 바라봐야할 위치. 월드 좌표로 오겠지만 애님인스턴스나 컨트롤릭에서 쓰려면 메쉬 스페이스로 변환해줘야함.
		_targetPos_World = _pawn->GetFocusingLoc();

		_targetPos_Mesh = _pawn->GetMesh()->GetComponentTransform().InverseTransformPosition(_targetPos_World);
		
		_aimPitch = UKismetMathLibrary::NormalizeAxis(_pawn->GetBaseAimRotation().Pitch);

		GetCharacterState(DeltaSeconds);


	}
}

void UCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);


	GetVelocityData();
	GetAccelerationData();
	GetLocationData();
	GetRotationData(DeltaSeconds);
	UpdateOrientationData();
	UpdateRootYawOffset(DeltaSeconds);

}



float  UCharacterAnimInstance::PlayAnimMontage(UAnimMontage* animMontage, float playRate )
{
	if (!Montage_IsPlaying(animMontage))
	{
		return Montage_Play(animMontage,playRate);
	}
	return 0.0f;
}


void UCharacterAnimInstance::JumpToSection(int32 sectionIndex)
{
	FName sectionName = FName(*FString::Printf(TEXT("Section%d"), sectionIndex));
	Montage_JumpToSection(sectionName);
}

bool UCharacterAnimInstance::IsMoving()
{
	_isMoving= 0.1 <= (_vertical * _vertical + _horizontal * _horizontal);
	return _isMoving;
}

void UCharacterAnimInstance::AnimNotify_Attack()
{

}




UCharacterMovementComponent* UCharacterAnimInstance::GetCharacterMovement()
{
	return TryGetCBOwner()->GetCharacterMovement();
}

ACharacterBase* UCharacterAnimInstance::TryGetCBOwner()
{

	if (_pawn == nullptr)
	{

		_pawn = Cast<ACharacterBase>(TryGetPawnOwner());
	}
	return _pawn;
}

FTransform UCharacterAnimInstance::GetAimBone()
{
	if (TryGetCBOwner() == nullptr)
		return FTransform();

	// 1. 메시 컴포넌트가 유효한지 먼저 확인합니다.
	if (TryGetCBOwner()->GetMesh())
	{
		// 2. 본의 인덱스를 가져옵니다.
		int32 BoneIndex = _pawn->GetMesh()->GetBoneIndex(_aimBoneName);

		// 3. 인덱스가 INDEX_NONE이 아니라면 유효한 본입니다.
		if (BoneIndex != INDEX_NONE)
		{

			return _pawn->GetMesh()->GetBoneTransform(_aimBoneName);
		}
		else
		{
			return _pawn->GetTransform();
		}
	}
	return TryGetCBOwner()->GetTransform();
}

void UCharacterAnimInstance::GetVelocityData()
{
	_characterVelocity2D = _characterVelocity * FVector(1.f, 1.f, 0.f);
}

void UCharacterAnimInstance::GetAccelerationData()
{
	_acceleration2D = _acceleration * FVector(1.f, 1.f, 0.f);

	_bIsAccelerating = !FMath::IsNearlyEqual(_acceleration2D.Size2D(), 0.f);
	

}

void UCharacterAnimInstance::GetLocationData()
{
	FVector temp = _worldLocation - _lastFrameWorldLocation;
	_deltaLocation = temp.Length();
}

void UCharacterAnimInstance::GetRotationData(float deltaTime)
{
	// 1. 회전 속도(Delta) 계산을 위해 이전 프레임의 Yaw 값을 미리 저장해 둡니다.
	_lastFrameActorYaw = _actorYaw;


	FTransform aimTransform = GetAimBone();
	// 2. 시선의 시작점을 캐릭터 위치(혹은 머리/총구 소켓 위치)로 설정합니다.
	FVector CharacterAimStart = aimTransform.GetLocation();

	// 3. 시작점에서 목표 지점을 바라보기 위한 절대적인 월드 회전값(LookAt)을 구합니다.
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(CharacterAimStart, _targetPos_World);

	// 4. 캐릭터의 현재 몸통 방향을 기준으로 목표 지점이 상하좌우로 얼마나 꺾여있는지 상대 각도(-180 ~ 180도)를 구합니다.
	FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(LookAtRotation, _worldRotation);

	// 5. [순서 변경] 보간을 하기 전에, 목표로 하는 상하 각도(Pitch)와 좌우 각도(Yaw)가 한계치(-90도 ~ 90도)를 넘지 않도록 먼저 제한(Clamp)합니다.
	// 이렇게 해야 캐릭터가 고개를 돌릴 수 없는 등 뒤의 각도를 향해 억지로 회전을 시도하지 않습니다.
	float TargetPitch = FMath::Clamp(DeltaRotator.Pitch, -90.0f, 90.0f);
	float TargetYaw = FMath::Clamp(DeltaRotator.Yaw, -90.0f, 90.0f);

	// 6. [순서 변경] 현재 각도에서, 방금 제한(Clamp)을 걸어둔 안전한 목표 각도(Target)로 부드럽게 보간(Interp)하여 값을 갱신합니다.
	// 5.0f는 고개가 돌아가는 속도입니다. 이 값을 조절하여 타격감에 맞는 속도를 찾으시면 됩니다.
	_aimPitch = FMath::FInterpTo(_aimPitch, TargetPitch, deltaTime, 5.0f);
	_actorYaw = FMath::FInterpTo(_actorYaw, TargetYaw, deltaTime, 5.0f);

	// 7. [순서 변경] 최종적으로 Clamp와 Interp가 모두 적용된 안전한 현재 프레임의 Yaw 값과, 
	// 아까 1번에서 저장해둔 이전 프레임의 Yaw 값의 차이를 구하여 실제 회전 변화량을 도출합니다.
	_deltaActorYaw = _actorYaw - _lastFrameActorYaw;

	// 8. 구한 변화량을 델타 타임으로 나누어 초당 회전 속도를 구하고, 필요한 스케일(여기서는 5)로 나누어 임시 변수에 담습니다.
	float temp = _deltaActorYaw / deltaTime;
	temp /= 5;
}
void UCharacterAnimInstance::UpdateOrientationData()
{
	_lastFrameVelocityLocomotionDirection = _velocityLocomotionDirection;

	_velocityLocomotionAngle = CalculateDirection(_characterVelocity2D, _worldRotation);

	_velocityLocomotionAngleWithOffset= UKismetMathLibrary::NormalizeAxis(_velocityLocomotionAngle - _rootYawOffset);

	_accelerationLocomotionAngle = CalculateDirection(_acceleration2D, _worldRotation);

	_velocityLocomotionDirection = CalculateLocomotionDirection(_velocityLocomotionAngle, _velocityLocomotionDirection);
	_accelerationLocomotionDirection = CalculateLocomotionDirection(_accelerationLocomotionAngle, _accelerationLocomotionDirection);
}

ELocomotionDirections UCharacterAnimInstance::CalculateLocomotionDirection(float currentLocomotionAngle, ELocomotionDirections currentDirection, float backwardMin, float backwardMax, float forwardMin, float forwardMax, float deadZone)
{

	switch (currentDirection)
	{
	case ELocomotionDirections::Forward:
	{
		if (UKismetMathLibrary::InRange_FloatFloat(currentLocomotionAngle, forwardMin - deadZone, forwardMax + deadZone))
			return ELocomotionDirections::Forward;

	}
		break;
	case ELocomotionDirections::Backward:
	{
		if (currentLocomotionAngle > backwardMax - deadZone || currentLocomotionAngle < backwardMin + deadZone)
			return ELocomotionDirections::Backward;

	}
		break;
	case ELocomotionDirections::Right:
	{
		if (UKismetMathLibrary::InRange_FloatFloat(currentLocomotionAngle, forwardMax - deadZone, backwardMax + deadZone))
			return ELocomotionDirections::Right;

	}
		break;
	case ELocomotionDirections::Left:
	{
		if (UKismetMathLibrary::InRange_FloatFloat(currentLocomotionAngle, backwardMin - deadZone, forwardMin + deadZone))
			return ELocomotionDirections::Left;

	}
		break;
	default:
		break;
	}

	//리턴이 안됬다면 상태가 바뀐것.

	if (currentLocomotionAngle > backwardMax  || currentLocomotionAngle < backwardMin )
		return ELocomotionDirections::Backward;
	if (UKismetMathLibrary::InRange_FloatFloat(currentLocomotionAngle, forwardMin , forwardMax ))
		return ELocomotionDirections::Forward;
	if(currentLocomotionAngle>0)
		return ELocomotionDirections::Right;

	return ELocomotionDirections::Left;



}

void UCharacterAnimInstance::GetCharacterState(float deltaTime)
{
	//상태변환 이전 프레임과 상태가 변화하면 알수 있음. 
	//이전프레임의 상태와 이번에들어온 상태, 현재상태를 갖고있음. 블루프린트에서 cpp로 옮기면서 절차가 살짝 바뀔수밖에 없었는데 그것때문에 문제가 생길수 있음.
	//
	_lastFrameState = _currentState;
	_currentState = _incommingState;
	_bIsStateChanged = _currentState != _lastFrameState;



	_bIsOnAir = _pawn->GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Falling;
	_bIsJumping = _bIsOnAir && _characterVelocity.Z > 0.f;
	_bIsFalling = _bIsOnAir && _characterVelocity.Z < 0.f;
	if (_bIsJumping)
	{
		//Todo : 최고점 도달 시간 설정. 0으로 나누면 안되는데 이걸 어떻게 설정해야할까 예외처리시 어떻게 해야할까 최대값으로 해야할까 0으로해야할까 
		float temp = _pawn->GetCharacterMovement()->GetGravityZ() * _pawn->GetCharacterMovement()->GravityScale;
		if ( FMath::Abs(temp) > 0.1f)
		{
			_timeToJumpApex = UKismetMathLibrary::SafeDivide( _characterVelocity.Z , temp);

		}
		else
		{
			//다음 스테이트로넘어가기위한 임계치보다 내려가지만 않으면 문제가 없다. 1로고정하자.
			_timeToJumpApex = 1.f;
		}

	}
	else
	{
		_timeToJumpApex = 0.f;
	}

	if (_bIsFalling)
	{
		_timeFalling = _timeFalling + deltaTime;

	}
	else
	{
		if (_bIsJumping)
			_timeFalling = 0.f;
	}
	
		
}

void UCharacterAnimInstance::UpdateRootYawOffset(float deltaTime)
{
	if (_rootYawOffsetMode == ERootYawOffsetMode::Accumulate)
	{
		SetRootYawOffset(_rootYawOffset + (_deltaActorYaw * -1));
	}
	if (_rootYawOffsetMode == ERootYawOffsetMode::BlendOut)
	{
		SetRootYawOffset(UKismetMathLibrary::FloatSpringInterp(_rootYawOffset, 0.f, _rootYawOffsetSpringState, 0, 1, deltaTime, 1.f, 0.5f));
	}

	_rootYawOffsetMode = ERootYawOffsetMode::BlendOut;
}

void UCharacterAnimInstance::SetRootYawOffset(float angle)
{
	_rootYawOffset = UKismetMathLibrary::NormalizeAxis(angle);
}

void UCharacterAnimInstance::UpdateIdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	_rootYawOffsetMode = ERootYawOffsetMode::Accumulate;
	ProcessTurnYawCurve();
}

void UCharacterAnimInstance::ProcessTurnYawCurve()
{
	_lastFrameTurnYawCurveValue = _turnYawCurveValue;

	// 1. 턴 애니메이션이 종료되었거나 재생 중이 아니면 값 초기화
	if (GetCurveValue(TEXT("IsTurning")) < 1.f)
	{
		_turnYawCurveValue = 0.f;
		_lastFrameTurnYawCurveValue = 0.f;
		return;
	}

	// 2. 현재 프레임의 커브 값 도출
	_turnYawCurveValue = UKismetMathLibrary::SafeDivide(GetCurveValue(TEXT("root_rotation_Z")), GetCurveValue(TEXT("IsTurning")));

	// 첫 프레임 스킵 (Delta 값이 비정상적으로 튀는 것 방지)
	if (_lastFrameTurnYawCurveValue != 0.f)
	{
		// 3. 이번 틱에 커브가 지시하는 '회전 변화량(Delta)' 계산
		float DeltaYaw = _turnYawCurveValue - _lastFrameTurnYawCurveValue;

		// 4. 오버슛 판별을 위한 임시 변수 할당
		float PreviousOffset = _rootYawOffset;
		float NextOffset = PreviousOffset - DeltaYaw;

		// 5. [오버슛 체크] Offset이 0을 스쳐 지나가서 부호가 바뀌었는가?
		if (FMath::Sign(PreviousOffset) != FMath::Sign(NextOffset) || FMath::IsNearlyZero(PreviousOffset))
		{
			// ==========================================
			// [목표 도달 시점] 오버슛 방지!
			// ==========================================

			// 캡슐을 커브값(DeltaYaw)만큼 돌리지 말고, 딱 '남은 잔여 각도(PreviousOffset)' 만큼만 회전시켜 정확히 맞춥니다.
			if (APawn* OwningPawn = TryGetPawnOwner())
			{
				OwningPawn->AddActorWorldRotation(FRotator(0.f, PreviousOffset, 0.f));
			}

			// 허리 비틀림(Offset)은 깔끔하게 0으로 스냅 (함수 활용)
			SetRootYawOffset(0.f);
		}
		else
		{
			// ==========================================
			// [정상 회전 중] 
			// ==========================================

			// 허리 비틀림 값을 깎아냅니다. (회원님 기존 로직과 동일)
			SetRootYawOffset(NextOffset);

			// 깎아낸 만큼 캡슐(Actor)을 물리적으로 회전시킵니다.
			if (APawn* OwningPawn = TryGetPawnOwner())
			{
				OwningPawn->AddActorWorldRotation(FRotator(0.f, DeltaYaw, 0.f));
			}
		}
	}
}

void UCharacterAnimInstance::SetupPivotState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	_pivotAcceleration2D = _acceleration2D;
}

void UCharacterAnimInstance::SetupJumpApexPose(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	_timeFalling = 0.f;
}

void UCharacterAnimInstance::SetupStateTransition(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{



}

void UCharacterAnimInstance::FinishStateTransition(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
}

void UCharacterAnimInstance::RecieveGroundDistance(float dist) 
{
	_groundDistance = dist;
}

void UCharacterAnimInstance::RecieveCharacterStateChange(int32 n)
{
	if (n < 0 || n >= (int32)ECharacterState::MAX)
		return;
	_incommingState = (ECharacterState)n;
}
