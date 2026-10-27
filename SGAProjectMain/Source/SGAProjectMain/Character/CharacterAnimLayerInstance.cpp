// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimLayerInstance.h"
#include "Animation/AnimExecutionContext.h" 
#include "Animation/AnimNodeReference.h"    
#include "SequencePlayerLibrary.h"
#include "SequenceEvaluatorLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AnimCharacterMovementLibrary.h"
#include "AnimDistanceMatchingLibrary.h"
#include "AnimExecutionContextLibrary.h"
#include "Kismet/KismetMathLibrary.h"


void UCharacterAnimLayerInstance::NativeInitializeAnimation()
{
	SetBaseAnim();
}

void UCharacterAnimLayerInstance::SetBaseAnim()
{
	auto temp = GetOwningComponent()->GetAnimInstance();
	if (temp)
	{
		_baseAnimInst = Cast<UCharacterAnimInstance>(temp);
	}
}

UCharacterAnimInstance* UCharacterAnimLayerInstance::GetBaseAnim()
{
	if (_baseAnimInst == nullptr)
	{
		SetBaseAnim();
	}
	return _baseAnimInst;
}

void UCharacterAnimLayerInstance::IdleOnUpdate(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult conversionResult;
	auto result=USequencePlayerLibrary::ConvertToSequencePlayer(Node, conversionResult);
	UAnimSequenceBase* anim = nullptr;
	switch (GetBaseAnim()->GetCurrentState())
	{
	case ECharacterState::Standing:
	case ECharacterState::Sprinting:

		anim = _idleAnim;
		break;
	case ECharacterState::Crouching:
		anim = _crouchIdleAnim;
		break;
	case ECharacterState::Proning:
		anim = _proneIdleAnims.GetAnim(GetProneDirection());

		break;
	case ECharacterState::Knockdown:
		break;
	case ECharacterState::MAX:
		break;
	default:
		break;
	}

	USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, result, anim,0.2f);

}

void UCharacterAnimLayerInstance::CycleOnUpdate(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult conversionResult;
	auto result = USequencePlayerLibrary::ConvertToSequencePlayer(Node, conversionResult);
	UAnimSequenceBase* anim = nullptr;

	switch (GetBaseAnim()->GetCurrentState())
	{
	case ECharacterState::Standing:

		anim = _walkCycleAnims.GetAnim(_baseAnimInst->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Sprinting:

		anim = _jogCycleAnims.GetAnim(_baseAnimInst->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Crouching:

		anim = _crouchCycleAnims.GetAnim(_baseAnimInst->GetVelocityLocomotionDirection());
		
		break;
	case ECharacterState::Proning:

		anim = _proneCycleAnims.GetAnim(_baseAnimInst->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Knockdown:
		break;
	case ECharacterState::MAX:
		break;
	default:
		break;
	}

	USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, result, anim, 0.2f);

}

ELocomotionDirections UCharacterAnimLayerInstance::GetProneDirection()
{
	float value = UKismetMathLibrary::NormalizeAxis(GetBaseAnim()->_actorYaw);
	if (UKismetMathLibrary::InRange_FloatFloat(value, -45.f, 45.f))
	{
		_proneDirection= ELocomotionDirections::Forward;
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(value, -135, -45.f))
	{
		_proneDirection = ELocomotionDirections::Left;

	}
	else if (UKismetMathLibrary::InRange_FloatFloat(value, 45.f, 135.f))
	{
		_proneDirection = ELocomotionDirections::Right;

	}
	else
	{
		_proneDirection = ELocomotionDirections::Backward;

	}
	return _proneDirection;

}

void UCharacterAnimLayerInstance::SetupStopAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult conversionResult;
	auto result = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);
	UAnimSequenceBase* anim = nullptr;

	switch (GetBaseAnim()->GetCurrentState())
	{
	case ECharacterState::Standing:

		anim = _walkStopAnims.GetAnim(_baseAnimInst->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Sprinting:

		anim = _jogStopAnims.GetAnim(_baseAnimInst->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Crouching:

		anim = _crouchStopAnims.GetAnim(_baseAnimInst->GetVelocityLocomotionDirection());

		break;
	case ECharacterState::Proning:

		anim = _proneStopAnims.GetAnim(_baseAnimInst->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Knockdown:
		break;
	case ECharacterState::MAX:
		break;
	default:
		break;
	}

	USequenceEvaluatorLibrary::SetSequence(result, anim);
	USequenceEvaluatorLibrary::SetExplicitTime(result,0.f);

}

void UCharacterAnimLayerInstance::UpdateStopAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{

	EAnimNodeReferenceConversionResult conversionResult;
	auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);

	float stopDistance=UKismetMathLibrary::VSizeXY(     UAnimCharacterMovementLibrary::PredictGroundMovementStopLocation(
		GetBaseAnim()->_characterVelocity2D,                         // 현재 속도 전달
		GetBaseAnim()->GetCharacterMovement()->bUseSeparateBrakingFriction,      // 별도 마찰력 사용 여부 (컴포넌트 설정값)
		GetBaseAnim()->GetCharacterMovement()->BrakingFriction,                  // 브레이크 마찰력
		GetBaseAnim()->GetCharacterMovement()->GroundFriction,                   // 지면 마찰력
		GetBaseAnim()->GetCharacterMovement()->BrakingFrictionFactor,            // 마찰 인자
		GetBaseAnim()->GetCharacterMovement()->BrakingDecelerationWalking        // 걷기 상태의 감속도
	));
	if (stopDistance > 0.f)
	{
		UAnimDistanceMatchingLibrary::DistanceMatchToTarget(sequenceEvaluator, stopDistance, _distanceCurveName);
	}
	else
	{
		USequenceEvaluatorLibrary::AdvanceTime(Context, sequenceEvaluator, 1.f);
	}

}

void UCharacterAnimLayerInstance::SetupStartAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult conversionResult;
	auto result = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);
	UAnimSequenceBase* anim = nullptr;

	switch (GetBaseAnim()->GetCurrentState())
	{
	case ECharacterState::Standing:

		anim = _walkStartAnims.GetAnim(GetBaseAnim()->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Sprinting:

		anim = _jogStartAnims.GetAnim(GetBaseAnim()->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Crouching:

		anim = _crouchStartAnims.GetAnim(GetBaseAnim()->GetVelocityLocomotionDirection());

		break;
	case ECharacterState::Proning:

		anim = _proneStartAnims.GetAnim(GetBaseAnim()->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Knockdown:
		break;
	case ECharacterState::MAX:
		break;
	default:
		break;
	}

	USequenceEvaluatorLibrary::SetSequence(result, anim);
	USequenceEvaluatorLibrary::SetExplicitTime(result, 0.f);
}

void UCharacterAnimLayerInstance::UpdateStartAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{

	EAnimNodeReferenceConversionResult conversionResult;
	auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);

	UAnimSequenceBase* anim = nullptr;

	switch (GetBaseAnim()->GetCurrentState())
	{
	case ECharacterState::Standing:

		anim = _walkStartAnims.GetAnim(GetBaseAnim()->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Sprinting:

		anim = _jogStartAnims.GetAnim(GetBaseAnim()->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Crouching:

		anim = _crouchStartAnims.GetAnim(GetBaseAnim()->GetVelocityLocomotionDirection());

		break;
	case ECharacterState::Proning:

		anim = _proneStartAnims.GetAnim(GetBaseAnim()->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Knockdown:
		break;
	case ECharacterState::MAX:
		break;
	default:
		break;
	}
	//사용하는 애니메이션이 달라졌다면 바꾸고 시간을 초기화하고 진행.
	if (USequenceEvaluatorLibrary::GetSequence(sequenceEvaluator) != anim)
	{

		USequenceEvaluatorLibrary::SetSequence(sequenceEvaluator, anim);
		USequenceEvaluatorLibrary::SetExplicitTime(sequenceEvaluator, 0.f);
	}
	UAnimDistanceMatchingLibrary::AdvanceTimeByDistanceMatching(Context, sequenceEvaluator, GetBaseAnim()->_deltaLocation, _distanceCurveName);
}

UAnimSequenceBase* UCharacterAnimLayerInstance::SelectedPivotAnim()
{
	UAnimSequenceBase* anim = nullptr;

	switch (GetBaseAnim()->GetCurrentState())
	{
	case ECharacterState::Standing:

		anim = _walkPivotAnims.GetAnim(_baseAnimInst->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Sprinting:

		anim = _jogPivotAnims.GetAnim(_baseAnimInst->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Crouching:

		anim = _crouchPivotAnims.GetAnim(_baseAnimInst->GetVelocityLocomotionDirection());

		break;
	case ECharacterState::Proning:

		anim = _pronePivotAnims.GetAnim(_baseAnimInst->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Knockdown:
		break;
	case ECharacterState::MAX:
		break;
	default:
		break;
	}
	return anim;
}

void UCharacterAnimLayerInstance::SetupPivotAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{

	EAnimNodeReferenceConversionResult conversionResult;
	auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);
	UAnimSequenceBase* anim = SelectedPivotAnim();

	USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, sequenceEvaluator, anim, 0.2f);
	USequenceEvaluatorLibrary::SetExplicitTime(sequenceEvaluator, 0.f);
}

void UCharacterAnimLayerInstance::UpdatePivotAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{

	EAnimNodeReferenceConversionResult conversionResult;
	auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);
	FVector velocity = GetBaseAnim()->_characterVelocity2D.GetSafeNormal();
	FVector acceleration = GetBaseAnim()->_acceleration2D.GetSafeNormal();
	if (FVector::DotProduct(velocity, acceleration)<0.f)
	{
		UAnimSequenceBase* anim = SelectedPivotAnim();
		if (USequenceEvaluatorLibrary::GetSequence(sequenceEvaluator) != anim)
		{
			USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, sequenceEvaluator, anim);
		}
		float distance = UAnimCharacterMovementLibrary::PredictGroundMovementPivotLocation(GetBaseAnim()->_acceleration2D,GetBaseAnim()->_characterVelocity2D,GetBaseAnim()->GetCharacterMovement()->GroundFriction).Size();
		UAnimDistanceMatchingLibrary::DistanceMatchToTarget(sequenceEvaluator, distance, _distanceCurveName);
	}
	else
	{
		UAnimDistanceMatchingLibrary::AdvanceTimeByDistanceMatching(Context, sequenceEvaluator, GetBaseAnim()->_deltaLocation, _distanceCurveName);
	}
	
	
}

void UCharacterAnimLayerInstance::SetupTurnInPlaceEntryState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	_bShouldTurnLeft = UKismetMathLibrary::SignOfFloat(GetBaseAnim()->_rootYawOffset) > 0.f;
}

void UCharacterAnimLayerInstance::SetupTurnInPlaceEntryAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{

	EAnimNodeReferenceConversionResult conversionResult;
	auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);
	_finalTurnAnim = SelectTurnInPlaceAnimation();

	USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, sequenceEvaluator, _finalTurnAnim);
	USequenceEvaluatorLibrary::SetExplicitTime(sequenceEvaluator, 0.f);
	_turnInPlaceTime = 0.f;
}

UAnimSequenceBase* UCharacterAnimLayerInstance::SelectTurnInPlaceAnimation()
{
	
	switch (GetBaseAnim()->GetCurrentState())
	{
	case ECharacterState::Standing:
	case ECharacterState::Sprinting:
	{
		if (UKismetMathLibrary::InRange_FloatFloat(FMath::Abs(UKismetMathLibrary::NormalizeAxis(GetBaseAnim()->_rootYawOffset)), 90.f, 180.f))
		{
			if (_bShouldTurnLeft)
				return _turnLeft180;
			else
				return _turnRight180;
		}
		else
		{
			if (_bShouldTurnLeft)
				return _turnLeft90;
			else
				return _turnRight90;

		}
	}
		break;
	case ECharacterState::Crouching:
	{
		if (UKismetMathLibrary::InRange_FloatFloat(FMath::Abs(UKismetMathLibrary::NormalizeAxis(GetBaseAnim()->_rootYawOffset)), 90.f, 180.f))
		{
			if (_bShouldTurnLeft)
				return _crouchTurnLeft180;
			else
				return _crouchTurnRight180;
		}
		else
		{
			if (_bShouldTurnLeft)
				return _crouchTurnLeft90;
			else
				return _crouchTurnRight90;

		}
	}
		break;
	case ECharacterState::Proning:
	{
		if (UKismetMathLibrary::InRange_FloatFloat(FMath::Abs(UKismetMathLibrary::NormalizeAxis(GetBaseAnim()->_rootYawOffset)), 90.f, 180.f))
		{
			if (_bShouldTurnLeft)
				return _proneTurnLeft180;
			else
				return _proneTurnRight180;
		}
		else
		{
			if (_bShouldTurnLeft)
				return _proneTurnLeft90;
			else
				return _proneTurnRight90;

		}
	}
		break;
	case ECharacterState::Knockdown:
		break;
	case ECharacterState::MAX:
		break;
	default:
		break;
	}
	return nullptr;
}

void UCharacterAnimLayerInstance::UpdateTurnInPlaceEntryAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{

	float deltaTime= UAnimExecutionContextLibrary::GetDeltaTime(Context);
	EAnimNodeReferenceConversionResult conversionResult;
	auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);
	_turnInPlaceTime += deltaTime;
	USequenceEvaluatorLibrary::SetExplicitTime(sequenceEvaluator, _turnInPlaceTime);
}

void UCharacterAnimLayerInstance::SetupJumpFallLandAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{

	EAnimNodeReferenceConversionResult conversionResult;
	auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);

	USequenceEvaluatorLibrary::SetExplicitTime(sequenceEvaluator, 0.f);
}

void UCharacterAnimLayerInstance::UpdateJumpFallLandAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{

	EAnimNodeReferenceConversionResult conversionResult;
	auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);

	UAnimDistanceMatchingLibrary::DistanceMatchToTarget(sequenceEvaluator, GetBaseAnim()->_groundDistance, _distanceCurveName);

}
void UCharacterAnimLayerInstance::SetupDodgeStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult conversionResult;
	auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);
	UAnimSequenceBase* anim = _dodgeStartAnims.GetAnim( GetProneDirection());

	USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, sequenceEvaluator, anim, 0.2f);
	USequenceEvaluatorLibrary::SetExplicitTime(sequenceEvaluator, 0.f);
}

void UCharacterAnimLayerInstance::UpdateDodgeStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult conversionResult;
	auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);
	UAnimSequenceBase* anim = _dodgeStartAnims.GetAnim(GetProneDirection());

	//사용하는 애니메이션이 달라졌다면 바꾸고 시간을 유지하고 진행.
	if (USequenceEvaluatorLibrary::GetSequence(sequenceEvaluator) != anim)
	{
		USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, sequenceEvaluator, anim, 0.2f);
	}


	UAnimDistanceMatchingLibrary::AdvanceTimeByDistanceMatching(Context, sequenceEvaluator, GetBaseAnim()->_deltaLocation, _distanceCurveName);
}

void UCharacterAnimLayerInstance::SetupDodgeLoopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult conversionResult;

	auto sequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, conversionResult);

	UAnimSequenceBase* anim = _dodgeLoopAnims.GetAnim(GetProneDirection());

	USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, sequencePlayer, anim, 0.2f);

	USequencePlayerLibrary::SetAccumulatedTime(sequencePlayer, 0.f);
}

void UCharacterAnimLayerInstance::UpdateDodgeLoopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult conversionResult;

	auto sequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, conversionResult);

	UAnimSequenceBase* anim = _dodgeLoopAnims.GetAnim(GetProneDirection());

	if (USequencePlayerLibrary::GetSequencePure(sequencePlayer) != anim)
	{

		USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, sequencePlayer, anim, 0.2f);
	}

}

void UCharacterAnimLayerInstance::SetupDodgeEndAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult conversionResult;
	auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);

	UAnimSequenceBase* anim = _dodgeEndAnims.GetAnim(GetProneDirection());

	USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, sequenceEvaluator, anim, 0.2f);
	USequenceEvaluatorLibrary::SetExplicitTime(sequenceEvaluator, 0.f);
}

void UCharacterAnimLayerInstance::UpdateDodgeEndAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{

	EAnimNodeReferenceConversionResult conversionResult;
	auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);

	UAnimSequenceBase* anim = _dodgeEndAnims.GetAnim(GetProneDirection());

	//사용하는 애니메이션이 달라졌다면 바꾸고 시간을 유지하고 진행.
	if (USequenceEvaluatorLibrary::GetSequence(sequenceEvaluator) != anim)
	{
		USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, sequenceEvaluator, anim, 0.2f);
	}

	UAnimDistanceMatchingLibrary::DistanceMatchToTarget(sequenceEvaluator, GetBaseAnim()->_groundDistance, _distanceCurveName);
}




void UCharacterAnimLayerInstance::SetupStanceTransitionAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult conversionResult;
	auto sequencePlayer=USequencePlayerLibrary::ConvertToSequencePlayer(Node, conversionResult);

	UAnimSequenceBase* anim = nullptr;


	switch (GetBaseAnim()->_incommingState)
	{
	case ECharacterState::Standing:
	case ECharacterState::Sprinting:
	{
		switch (GetBaseAnim()->_lastFrameState)
		{
		case ECharacterState::Standing:
		case ECharacterState::Sprinting:
			break;
		case ECharacterState::Crouching:
			anim = _crouchToStandAnim;
			break;
		case ECharacterState::Proning:
			anim = _proneToStandAnim;
			break;
		case ECharacterState::Knockdown:
		case ECharacterState::MAX:
		default:
			break;
		}
	}
		break;
	case ECharacterState::Crouching:
	{
		switch (GetBaseAnim()->_lastFrameState)
		{
		case ECharacterState::Standing:
		case ECharacterState::Sprinting:
			anim = _standToCrouchAnim;
			break;
		case ECharacterState::Crouching:
			break;
		case ECharacterState::Proning:
			anim = _proneToCrouchAnim;
			break;
		case ECharacterState::Knockdown:
		case ECharacterState::MAX:
		default:
			break;
		}
	}
		break;
	case ECharacterState::Proning:
	{
		switch (GetBaseAnim()->_lastFrameState)
		{
		case ECharacterState::Standing:
		case ECharacterState::Sprinting:
			anim = _standToProneAnim;
			break;
		case ECharacterState::Crouching:
			anim = _crouchToProneAnim;
			break;
		case ECharacterState::Proning:
			break;
		case ECharacterState::Knockdown:
		case ECharacterState::MAX:
		default:
			break;
		}
	}
		break;
	case ECharacterState::Knockdown:
		break;
	case ECharacterState::MAX:
		break;
	default:
		break;
	}



	USequencePlayerLibrary::SetSequence(sequencePlayer, anim);



}
