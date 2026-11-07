// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"
#include "CharacterAimMath.h"

#include "CharacterBase.h"
#include "PlayerCharacter.h"
#include "CharacterStateComponent.h"
#include "HellDiver/HellDiverStateComponent.h"
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


	_pawn = Cast<ACharacterBase>(TryGetPawnOwner());
	_hasLocationSnapshot = false;
	_proneSector = 0;
	_pronePoseDirection = ELocomotionDirections::Forward;
	_stanceTransitionSource = _currentState;
	_hasAimSnapshot = false;
	_hasObservedActorYaw = false;
	_hasValidAimDirection = false;
	_observedActorYawDelta = 0.f;
	_aimYawFullRange = 0.f;
    _stanceTransitionRequested = false;
    _stanceTransitionComplete = false;
    _stanceTransitionRequestId = 0;
    _stanceTransitionPlaying = false;
    _stanceTransitionEvaluated = false;

}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	ACharacterBase* currentOwner = Cast<ACharacterBase>(TryGetPawnOwner());
	if (!IsValid(currentOwner)) currentOwner = nullptr;
	if (_pawn != currentOwner)
	{
		_pawn = currentOwner;
		_hasLocationSnapshot = false;
		_hasObservedActorYaw = false;
		_rootYawOffset = 0.f;
	}

    _stanceTransitionEvaluated = false;
    _stanceTransitionMoving = _pawn && _pawn->GetVelocity().SizeSquared2D() > 9.f;
    const auto* StanceState = _pawn ? Cast<UHellDiverStateComponent>(_pawn->GetStateComponent()) : nullptr;
    if (StanceState && StanceState->IsMotionChanging())
    {
        if (_stanceTransitionRequestId != StanceState->GetMotionChangeId())
        {
            _stanceTransitionRequestId = StanceState->GetMotionChangeId();
            _stanceTransitionRequested = true;
            _stanceTransitionComplete = false;
            _stanceTransitionPlaying = false;
            _stanceTransitionSource = StanceState->GetMotionChangeSource();
        }
    }
    else
    {
        _stanceTransitionRequested = false;
        if (_stanceTransitionPlaying) _stanceTransitionComplete = true;
        _stanceTransitionPlaying = false;
    }
	_hasAimSnapshot = false;
	_observedActorYawDelta = 0.f;
	if (!_pawn)
	{
		_hasObservedActorYaw = false;
		_hasLocationSnapshot = false;
		_hasValidAimDirection = false;
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
		

		const FVector currentLocation = _pawn->GetActorLocation();
		_lastFrameWorldLocation = _hasLocationSnapshot ? _worldLocation : currentLocation;
		_worldLocation = currentLocation;
		_hasLocationSnapshot = true;

		_worldRotation = _pawn->GetActorRotation();

		const float currentActorWorldYaw = static_cast<float>(_worldRotation.Yaw);
		_observedActorYawDelta = _hasObservedActorYaw
			? CharacterAimMath::CalculateActorYawDelta(_previousObservedActorYaw, currentActorWorldYaw)
			: 0.f;
		_previousObservedActorYaw = currentActorWorldYaw;
		_hasObservedActorYaw = true;
		//타겟이 바라봐야할 위치. 월드 좌표로 오겠지만 애님인스턴스나 컨트롤릭에서 쓰려면 메쉬 스페이스로 변환해줘야함.
		_targetPos_World = _pawn->GetFocusingLoc();

		_targetPos_Mesh = _pawn->GetMesh()->GetComponentTransform().InverseTransformPosition(_targetPos_World);

		// Capture the bone position on the game thread; worker updates use values only.
		_aimOriginWorld = GetAimBone().GetLocation();
		_hasAimSnapshot = true;
		
		// _aimPitch retains its filtered result between animation updates.

		_incommingState = _pawn->GetStateComponent()->GetCharacterState();
		GetCharacterState(DeltaSeconds);


	}
}

void UCharacterAnimInstance::NativePostEvaluateAnimation()
{
    Super::NativePostEvaluateAnimation();
    // Worker node callbacks report values only. Gameplay locks are changed here on the game thread.
    auto* State = _pawn ? Cast<UHellDiverStateComponent>(_pawn->GetStateComponent()) : nullptr;
    if (_stanceTransitionPlaying && (_stanceTransitionComplete || !_stanceTransitionEvaluated))
    {
        if (State) State->FinishMotionChange(_stanceTransitionRequestId);
        _stanceTransitionComplete = true;
        _stanceTransitionRequested = false;
        _stanceTransitionPlaying = false;
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
	if (!_hasLocationSnapshot)
	{
		_deltaLocation = 0.f;
		return;
	}
	FVector temp = _worldLocation - _lastFrameWorldLocation;
	const float horizontalDistance = temp.Size2D();
	// Spawning and teleporting must not advance a distance-matched clip.
	_deltaLocation = FMath::IsFinite(horizontalDistance) && horizontalDistance < 1000.f
		? horizontalDistance : 0.f;
}

void UCharacterAnimInstance::GetRotationData(float deltaTime)
{
	_lastFrameActorYaw = _actorYaw;
	_deltaActorYaw = _observedActorYawDelta;
	CharacterAimMath::FAimAngles aimAngles;
	_hasValidAimDirection = _hasAimSnapshot && CharacterAimMath::TryCalculateAimAngles(
		_aimOriginWorld, _targetPos_World, _worldRotation, aimAngles);
	if (!_hasValidAimDirection)
	{
		return;
	}

	_aimYawFullRange = aimAngles.Yaw;
	// Existing prone AO samples use absolute actor-relative yaw (right=90, rear=180).
	_actorYaw = _currentState == ECharacterState::Proning
		? CharacterAimMath::InterpolateFullAimYaw(_actorYaw, aimAngles.Yaw, deltaTime)
		: CharacterAimMath::InterpolateLimitedAimAngle(_actorYaw, aimAngles.Yaw, deltaTime);
	_aimPitch = CharacterAimMath::InterpolateLimitedAimAngle(_aimPitch, aimAngles.Pitch, deltaTime);

	// Pose selection and the additive AO consume the same filtered heading.
	const float proneHeading = _currentState == ECharacterState::Proning ? _actorYaw : _aimYawFullRange;
	_proneSector = CharacterAimMath::SelectProneSector(proneHeading, _proneSector);
	static const ELocomotionDirections directions[] = {
		ELocomotionDirections::Forward, ELocomotionDirections::Right,
		ELocomotionDirections::Backward, ELocomotionDirections::Left };
	_pronePoseDirection = directions[_proneSector];
	// Diagnostic only: do not feed residual coordinates to absolute-yaw AO assets.
	_proneResidualYaw = CharacterAimMath::CalculateProneResidualYaw(proneHeading, _proneSector);
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

void UCharacterAnimInstance::UpdateStanceState()
{
	_lastFrameState = _currentState;
	if (_currentState != _incommingState)
	{
		_stanceTransitionSource = _currentState;
	}
	_currentState = _incommingState;
	_bIsStateChanged = _currentState != _lastFrameState;

}

void UCharacterAnimInstance::GetCharacterState(float deltaTime)
{
	//상태변환 이전 프레임과 상태가 변화하면 알수 있음. 
	//이전프레임의 상태와 이번에들어온 상태, 현재상태를 갖고있음. 블루프린트에서 cpp로 옮기면서 절차가 살짝 바뀔수밖에 없었는데 그것때문에 문제가 생길수 있음.
	//
	UpdateStanceState();



	_bIsOnAir = _pawn->GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Falling;
	_bIsJumping = _bIsOnAir && _characterVelocity.Z > 0.f;
	_bIsFalling = _bIsOnAir && _characterVelocity.Z < 0.f;
	if (_bIsJumping)
	{
		//Todo : 최고점 도달 시간 설정. 0으로 나누면 안되는데 이걸 어떻게 설정해야할까 예외처리시 어떻게 해야할까 최대값으로 해야할까 0으로해야할까 
		float temp = _pawn->GetCharacterMovement()->GetGravityZ();
		if ( FMath::Abs(temp) > 0.1f)
		{
			_timeToJumpApex = UKismetMathLibrary::SafeDivide(_characterVelocity.Z, -temp);

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
	// Prone direction is represented by the body pose, not a standing root twist.
	if (_currentState == ECharacterState::Proning)
	{
		SetRootYawOffset(0.f);
		_rootYawOffsetSpringState.Reset();
		_rootYawOffsetMode = ERootYawOffsetMode::Hold;
		return;
	}
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
	const float turningWeight = GetCurveValue(TEXT("IsTurning"));
	if (_currentState == ECharacterState::Proning || turningWeight < 1.f)
	{
		_turnYawCurveValue = 0.f;
		_lastFrameTurnYawCurveValue = 0.f;
		return;
	}
	_turnYawCurveValue = GetCurveValue(TEXT("root_rotation_Z")) / turningWeight;
	if (!FMath::IsNearlyZero(_lastFrameTurnYawCurveValue))
	{
		// This callback can run on a worker. Movement owns the actor transform.
		SetRootYawOffset(CharacterAimMath::ConsumeTurnCurve(
			_rootYawOffset, _turnYawCurveValue - _lastFrameTurnYawCurveValue));
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
