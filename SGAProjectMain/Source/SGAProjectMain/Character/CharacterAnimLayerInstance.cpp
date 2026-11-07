// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimLayerInstance.h"
#include "HellDiver/HellDiver.h"
#include "HellDiver/HellDiverStateComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimCurveCompressionCodec_UniformIndexable.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
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
    Super::NativeInitializeAnimation();
    _hasDodgeDirection = false;
    _proneDirection = ELocomotionDirections::Forward;
    _predictedStopDistance = 0.f;
    _predictedPivotDistance = 0.f;
    SetBaseAnim();
}

void UCharacterAnimLayerInstance::SetBaseAnim()
{
    const USkeletalMeshComponent* component = GetOwningComponent();
    UCharacterAnimInstance* const base = component ? Cast<UCharacterAnimInstance>(component->GetAnimInstance()) : nullptr;
    if (_baseAnimInst != base) _hasDodgeDirection = false;
    _baseAnimInst = base;
}

UCharacterAnimInstance* UCharacterAnimLayerInstance::GetBaseAnim()
{
    // NativeInitialize/NativeUpdate prepare this cache on the game thread.
    return _baseAnimInst;
}

void UCharacterAnimLayerInstance::IdleOnUpdate(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
    UCharacterAnimInstance* const base = GetBaseAnim();
    if (!base) return;

	EAnimNodeReferenceConversionResult conversionResult;
	auto result=USequencePlayerLibrary::ConvertToSequencePlayer(Node, conversionResult);
    if (conversionResult != EAnimNodeReferenceConversionResult::Succeeded) return;
	UAnimSequenceBase* anim = nullptr;
	switch (base->GetCurrentState())
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

    if (!anim) anim = SelectIdleAnimation(base);
    if (!anim) return;
    USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, result, anim,0.2f);
}

void UCharacterAnimLayerInstance::CycleOnUpdate(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
    UCharacterAnimInstance* const base = GetBaseAnim();
    if (!base) return;

	EAnimNodeReferenceConversionResult conversionResult;
	auto result = USequencePlayerLibrary::ConvertToSequencePlayer(Node, conversionResult);
    if (conversionResult != EAnimNodeReferenceConversionResult::Succeeded) return;
	UAnimSequenceBase* anim = nullptr;

	switch (base->GetCurrentState())
	{
	case ECharacterState::Standing:

		anim = _walkCycleAnims.GetAnim(base->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Sprinting:

		anim = _jogCycleAnims.GetAnim(base->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Crouching:

		anim = _crouchCycleAnims.GetAnim(base->GetVelocityLocomotionDirection());
		
		break;
	case ECharacterState::Proning:

		anim = _proneCycleAnims.GetAnim(base->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Knockdown:
		break;
	case ECharacterState::MAX:
		break;
	default:
		break;
	}

    if (!anim) anim = SelectIdleAnimation(base);
    if (!anim) return;
    USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, result, anim, 0.2f);
}

ELocomotionDirections UCharacterAnimLayerInstance::GetProneDirection()
{
    if (UCharacterAnimInstance* base = GetBaseAnim())
        _proneDirection = base->GetPronePoseDirection();
    return _proneDirection;
}

void UCharacterAnimLayerInstance::SetupStopAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
    UCharacterAnimInstance* const base = GetBaseAnim();
    if (!base) return;

	EAnimNodeReferenceConversionResult conversionResult;
	auto result = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);
    if (conversionResult != EAnimNodeReferenceConversionResult::Succeeded) return;
	UAnimSequenceBase* anim = nullptr;

	switch (base->GetCurrentState())
	{
	case ECharacterState::Standing:

		anim = _walkStopAnims.GetAnim(base->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Sprinting:

		anim = _jogStopAnims.GetAnim(base->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Crouching:

		anim = _crouchStopAnims.GetAnim(base->GetVelocityLocomotionDirection());

		break;
	case ECharacterState::Proning:

		anim = _proneStopAnims.GetAnim(base->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Knockdown:
		break;
	case ECharacterState::MAX:
		break;
	default:
		break;
	}

    if (!anim) anim = SelectIdleAnimation(base);
    if (!anim) return;
    USequenceEvaluatorLibrary::SetSequence(result, anim);
	USequenceEvaluatorLibrary::SetExplicitTime(result,0.f);
}

void UCharacterAnimLayerInstance::UpdateStopAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
    UCharacterAnimInstance* const base = GetBaseAnim();
    if (!base) return;


	EAnimNodeReferenceConversionResult conversionResult;
	auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);
    if (conversionResult != EAnimNodeReferenceConversionResult::Succeeded) return;

	const float stopDistance = _predictedStopDistance;
    UAnimSequenceBase* stopAnim = SelectStopAnimation(base);
    if (!stopAnim) return;
    if (USequenceEvaluatorLibrary::GetSequence(sequenceEvaluator) != stopAnim)
    {
        USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, sequenceEvaluator, stopAnim, 0.2f);
        USequenceEvaluatorLibrary::SetExplicitTime(sequenceEvaluator, 0.f);
    }
    if (stopDistance > 0.f)
	{
		MatchDistanceOrTime(Context, sequenceEvaluator, stopDistance);
	}
	else
	{
		USequenceEvaluatorLibrary::AdvanceTime(Context, sequenceEvaluator, 1.f);
	}
}

void UCharacterAnimLayerInstance::SetupStartAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
    UCharacterAnimInstance* const base = GetBaseAnim();
    if (!base) return;

	EAnimNodeReferenceConversionResult conversionResult;
	auto result = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);
    if (conversionResult != EAnimNodeReferenceConversionResult::Succeeded) return;
	UAnimSequenceBase* anim = nullptr;

	switch (base->GetCurrentState())
	{
	case ECharacterState::Standing:

		anim = _walkStartAnims.GetAnim(base->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Sprinting:

		anim = _jogStartAnims.GetAnim(base->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Crouching:

		anim = _crouchStartAnims.GetAnim(base->GetVelocityLocomotionDirection());

		break;
	case ECharacterState::Proning:

		anim = _proneStartAnims.GetAnim(base->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Knockdown:
		break;
	case ECharacterState::MAX:
		break;
	default:
		break;
	}

    if (!anim) anim = SelectIdleAnimation(base);
    if (!anim) return;
    USequenceEvaluatorLibrary::SetSequence(result, anim);
	USequenceEvaluatorLibrary::SetExplicitTime(result, 0.f);
}

void UCharacterAnimLayerInstance::UpdateStartAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
    UCharacterAnimInstance* const base = GetBaseAnim();
    if (!base) return;


	EAnimNodeReferenceConversionResult conversionResult;
	auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);
    if (conversionResult != EAnimNodeReferenceConversionResult::Succeeded) return;

	UAnimSequenceBase* anim = nullptr;

	switch (base->GetCurrentState())
	{
	case ECharacterState::Standing:

		anim = _walkStartAnims.GetAnim(base->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Sprinting:

		anim = _jogStartAnims.GetAnim(base->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Crouching:

		anim = _crouchStartAnims.GetAnim(base->GetVelocityLocomotionDirection());

		break;
	case ECharacterState::Proning:

		anim = _proneStartAnims.GetAnim(base->GetVelocityLocomotionDirection());
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

        if (!anim) anim = SelectIdleAnimation(base);
        if (!anim) return;
        USequenceEvaluatorLibrary::SetSequence(sequenceEvaluator, anim);
		USequenceEvaluatorLibrary::SetExplicitTime(sequenceEvaluator, 0.f);
	}
	AdvanceDistanceOrTime(Context, sequenceEvaluator, base->_deltaLocation);
}

UAnimSequenceBase* UCharacterAnimLayerInstance::SelectedPivotAnim()
{
    UCharacterAnimInstance* const base = GetBaseAnim();
    if (!base) return nullptr;

	UAnimSequenceBase* anim = nullptr;

	switch (base->GetCurrentState())
	{
	case ECharacterState::Standing:

		anim = _walkPivotAnims.GetAnim(base->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Sprinting:

		anim = _jogPivotAnims.GetAnim(base->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Crouching:

		anim = _crouchPivotAnims.GetAnim(base->GetVelocityLocomotionDirection());

		break;
	case ECharacterState::Proning:

		anim = _pronePivotAnims.GetAnim(base->GetVelocityLocomotionDirection());
		break;
	case ECharacterState::Knockdown:
		break;
	case ECharacterState::MAX:
		break;
	default:
		break;
	}
	return anim ? anim : SelectIdleAnimation(base);
}

void UCharacterAnimLayerInstance::SetupPivotAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
    UCharacterAnimInstance* const base = GetBaseAnim();
    if (!base) return;


	EAnimNodeReferenceConversionResult conversionResult;
	auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);
    if (conversionResult != EAnimNodeReferenceConversionResult::Succeeded) return;
	UAnimSequenceBase* anim = SelectedPivotAnim();

    if (!anim) anim = SelectIdleAnimation(base);
    if (!anim) return;
    USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, sequenceEvaluator, anim, 0.2f);
	USequenceEvaluatorLibrary::SetExplicitTime(sequenceEvaluator, 0.f);
}

void UCharacterAnimLayerInstance::UpdatePivotAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
    UCharacterAnimInstance* const base = GetBaseAnim();
    if (!base) return;


	EAnimNodeReferenceConversionResult conversionResult;
	auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);
    if (conversionResult != EAnimNodeReferenceConversionResult::Succeeded) return;
	UAnimSequenceBase* pivotAnim = SelectedPivotAnim();
    if (!pivotAnim) return;
    if (USequenceEvaluatorLibrary::GetSequence(sequenceEvaluator) != pivotAnim)
    {
        USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, sequenceEvaluator, pivotAnim, 0.2f);
        USequenceEvaluatorLibrary::SetExplicitTime(sequenceEvaluator, 0.f);
    }
    FVector velocity = base->_characterVelocity2D.GetSafeNormal();
	FVector acceleration = base->_acceleration2D.GetSafeNormal();
	if (FVector::DotProduct(velocity, acceleration)<0.f)
	{
		UAnimSequenceBase* anim = SelectedPivotAnim();
		if (USequenceEvaluatorLibrary::GetSequence(sequenceEvaluator) != anim)
		{
            if (!anim) anim = SelectIdleAnimation(base);
            if (!anim) return;
            USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, sequenceEvaluator, anim);
		}
		float distance = _predictedPivotDistance;
		MatchDistanceOrTime(Context, sequenceEvaluator, distance);
	}
	else
	{
		AdvanceDistanceOrTime(Context, sequenceEvaluator, base->_deltaLocation);
	}
	
	
}

void UCharacterAnimLayerInstance::SetupTurnInPlaceEntryState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
    UCharacterAnimInstance* const base = GetBaseAnim();
    if (!base) return;

	_bShouldTurnLeft = UKismetMathLibrary::SignOfFloat(base->_rootYawOffset) > 0.f;
}

void UCharacterAnimLayerInstance::SetupTurnInPlaceEntryAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
    UCharacterAnimInstance* const base = GetBaseAnim();
    if (!base) return;


	EAnimNodeReferenceConversionResult conversionResult;
	auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);
    if (conversionResult != EAnimNodeReferenceConversionResult::Succeeded) return;
	_finalTurnAnim = SelectTurnInPlaceAnimation();

    if (!_finalTurnAnim) _finalTurnAnim = SelectIdleAnimation(base);
    if (!_finalTurnAnim) return;
    USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, sequenceEvaluator, _finalTurnAnim);
	USequenceEvaluatorLibrary::SetExplicitTime(sequenceEvaluator, 0.f);
	_turnInPlaceTime = 0.f;
}

UAnimSequenceBase* UCharacterAnimLayerInstance::SelectTurnInPlaceAnimation()
{
    UCharacterAnimInstance* const base = GetBaseAnim();
    if (!base) return nullptr;

	
	switch (base->GetCurrentState())
	{
	case ECharacterState::Standing:
	case ECharacterState::Sprinting:
	{
		if (UKismetMathLibrary::InRange_FloatFloat(FMath::Abs(UKismetMathLibrary::NormalizeAxis(base->_rootYawOffset)), 90.f, 180.f))
		{
			if (_bShouldTurnLeft)
				return _turnLeft180 ? _turnLeft180 : SelectIdleAnimation(base);
			else
				return _turnRight180 ? _turnRight180 : SelectIdleAnimation(base);
		}
		else
		{
			if (_bShouldTurnLeft)
				return _turnLeft90 ? _turnLeft90 : SelectIdleAnimation(base);
			else
				return _turnRight90 ? _turnRight90 : SelectIdleAnimation(base);

		}
	}
		break;
	case ECharacterState::Crouching:
	{
		if (UKismetMathLibrary::InRange_FloatFloat(FMath::Abs(UKismetMathLibrary::NormalizeAxis(base->_rootYawOffset)), 90.f, 180.f))
		{
			if (_bShouldTurnLeft)
				return _crouchTurnLeft180 ? _crouchTurnLeft180 : SelectIdleAnimation(base);
			else
				return _crouchTurnRight180 ? _crouchTurnRight180 : SelectIdleAnimation(base);
		}
		else
		{
			if (_bShouldTurnLeft)
				return _crouchTurnLeft90 ? _crouchTurnLeft90 : SelectIdleAnimation(base);
			else
				return _crouchTurnRight90 ? _crouchTurnRight90 : SelectIdleAnimation(base);

		}
	}
		break;
	case ECharacterState::Proning:
	{
		if (UKismetMathLibrary::InRange_FloatFloat(FMath::Abs(UKismetMathLibrary::NormalizeAxis(base->_rootYawOffset)), 90.f, 180.f))
		{
			if (_bShouldTurnLeft)
				return _proneTurnLeft180 ? _proneTurnLeft180 : SelectIdleAnimation(base);
			else
				return _proneTurnRight180 ? _proneTurnRight180 : SelectIdleAnimation(base);
		}
		else
		{
			if (_bShouldTurnLeft)
				return _proneTurnLeft90 ? _proneTurnLeft90 : SelectIdleAnimation(base);
			else
				return _proneTurnRight90 ? _proneTurnRight90 : SelectIdleAnimation(base);

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
    UCharacterAnimInstance* const base = GetBaseAnim();
    if (!base) return;


	if (!Context.GetContext()) return;
    float deltaTime= UAnimExecutionContextLibrary::GetDeltaTime(Context);
	EAnimNodeReferenceConversionResult conversionResult;
	auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);
    if (conversionResult != EAnimNodeReferenceConversionResult::Succeeded) return;
	if (!FMath::IsFinite(deltaTime) || deltaTime <= 0.f) return;
    _turnInPlaceTime += deltaTime;
	USequenceEvaluatorLibrary::SetExplicitTime(sequenceEvaluator, _turnInPlaceTime);
}

void UCharacterAnimLayerInstance::SetupJumpFallLandAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
    UCharacterAnimInstance* const base = GetBaseAnim();
    if (!base) return;


	EAnimNodeReferenceConversionResult conversionResult;
	auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);
    if (conversionResult != EAnimNodeReferenceConversionResult::Succeeded) return;

	if (!USequenceEvaluatorLibrary::GetSequence(sequenceEvaluator) && _jumpFallLand)
    USequenceEvaluatorLibrary::SetSequence(sequenceEvaluator, _jumpFallLand);
    if (!USequenceEvaluatorLibrary::GetSequence(sequenceEvaluator)) return;
    USequenceEvaluatorLibrary::SetExplicitTime(sequenceEvaluator, 0.f);
}

void UCharacterAnimLayerInstance::UpdateJumpFallLandAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
    UCharacterAnimInstance* const base = GetBaseAnim();
    if (!base) return;


	EAnimNodeReferenceConversionResult conversionResult;
	auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);
    if (conversionResult != EAnimNodeReferenceConversionResult::Succeeded) return;

	MatchDistanceOrTime(Context, sequenceEvaluator, base->_groundDistance);
}
void UCharacterAnimLayerInstance::SetupDodgeStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
    UCharacterAnimInstance* const base = GetBaseAnim();
    if (!base) return;
    _dodgeDirection = GetProneDirection();
    _hasDodgeDirection = true;
    UAnimSequenceBase* anim = _dodgeStartAnims.GetAnim(_dodgeDirection);
    if (!anim) anim = SelectIdleAnimation(base);
    if (!anim) return;
    EAnimNodeReferenceConversionResult conversionResult;
    auto sequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, conversionResult);
    if (conversionResult == EAnimNodeReferenceConversionResult::Succeeded)
    {
        USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, sequencePlayer, anim, 0.2f);
        USequencePlayerLibrary::SetAccumulatedTime(sequencePlayer, 0.f);
        return;
    }
    auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);
    if (conversionResult != EAnimNodeReferenceConversionResult::Succeeded) return;
    USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, sequenceEvaluator, anim, 0.2f);
    USequenceEvaluatorLibrary::SetExplicitTime(sequenceEvaluator, 0.f);
}

void UCharacterAnimLayerInstance::UpdateDodgeStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
    UCharacterAnimInstance* const base = GetBaseAnim();
    if (!base) return;
    EAnimNodeReferenceConversionResult conversionResult;
    USequencePlayerLibrary::ConvertToSequencePlayer(Node, conversionResult);
    if (conversionResult == EAnimNodeReferenceConversionResult::Succeeded) return;
    auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);
    if (conversionResult != EAnimNodeReferenceConversionResult::Succeeded) return;
    AdvanceDistanceOrTime(Context, sequenceEvaluator, base->_deltaLocation);
}

void UCharacterAnimLayerInstance::SetupDodgeLoopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
    UCharacterAnimInstance* const base = GetBaseAnim();
    if (!base) return;

	EAnimNodeReferenceConversionResult conversionResult;

	auto sequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, conversionResult);
    if (conversionResult != EAnimNodeReferenceConversionResult::Succeeded) return;

	if (!_hasDodgeDirection) { _dodgeDirection = GetProneDirection(); _hasDodgeDirection = true; }
    UAnimSequenceBase* anim = _dodgeLoopAnims.GetAnim(_dodgeDirection);

    if (!anim) anim = SelectIdleAnimation(base);
    if (!anim) return;
    USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, sequencePlayer, anim, 0.2f);

	USequencePlayerLibrary::SetAccumulatedTime(sequencePlayer, 0.f);
}

void UCharacterAnimLayerInstance::UpdateDodgeLoopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
    // The player advances its selected sequence; camera changes must not replace it mid-action.
    if (!GetBaseAnim()) return;
    EAnimNodeReferenceConversionResult conversionResult;
    USequencePlayerLibrary::ConvertToSequencePlayer(Node, conversionResult);
}

void UCharacterAnimLayerInstance::SetupDodgeEndAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
    UCharacterAnimInstance* const base = GetBaseAnim();
    if (!base) return;

	EAnimNodeReferenceConversionResult conversionResult;
	auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);
    if (conversionResult != EAnimNodeReferenceConversionResult::Succeeded) return;

	if (!_hasDodgeDirection) _dodgeDirection = GetProneDirection();
    UAnimSequenceBase* anim = _dodgeEndAnims.GetAnim(_dodgeDirection);
    _hasDodgeDirection = false;

    if (!anim) anim = SelectIdleAnimation(base);
    if (!anim) return;
    USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, sequenceEvaluator, anim, 0.2f);
	USequenceEvaluatorLibrary::SetExplicitTime(sequenceEvaluator, 0.f);
}

void UCharacterAnimLayerInstance::UpdateDodgeEndAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
    UCharacterAnimInstance* const base = GetBaseAnim();
    if (!base) return;
    EAnimNodeReferenceConversionResult conversionResult;
    auto sequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, conversionResult);
    if (conversionResult != EAnimNodeReferenceConversionResult::Succeeded) return;
    MatchDistanceOrTime(Context, sequenceEvaluator, base->_groundDistance);
}




UAnimSequenceBase* UCharacterAnimLayerInstance::SelectStanceTransitionAnimation(ECharacterState Source, ECharacterState Target) const
{
    const bool WasStanding = Source == ECharacterState::Standing || Source == ECharacterState::Sprinting;
    if (Target == ECharacterState::Standing || Target == ECharacterState::Sprinting)
        return Source == ECharacterState::Crouching ? _crouchToStandAnim : Source == ECharacterState::Proning ? _proneToStandAnim : nullptr;
    if (Target == ECharacterState::Crouching)
        return WasStanding ? _standToCrouchAnim : Source == ECharacterState::Proning ? _proneToCrouchAnim : nullptr;
    if (Target == ECharacterState::Proning)
        return WasStanding ? _standToProneAnim : Source == ECharacterState::Crouching ? _crouchToProneAnim : nullptr;
    return nullptr;
}

void UCharacterAnimLayerInstance::SetupStanceTransitionAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
    UCharacterAnimInstance* const Base = GetBaseAnim();
    if (!Base) return;
    EAnimNodeReferenceConversionResult Result;
    const auto Player = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);
    if (Result != EAnimNodeReferenceConversionResult::Succeeded) return;
    UAnimSequenceBase* Clip = SelectStanceTransitionAnimation(Base->GetStanceTransitionSource(), Base->GetCurrentState());
    Base->_stanceTransitionRequested = false;
    Base->_stanceTransitionPlaying = true;
    Base->_stanceTransitionEvaluated = true;
    Base->_stanceTransitionComplete = !Clip || Clip->GetPlayLength() <= 0.f;
    if (!Clip) Clip = SelectIdleAnimation(Base);
    USequencePlayerLibrary::SetSequence(Player, Clip);
    USequencePlayerLibrary::SetAccumulatedTime(Player, 0.f);
}

void UCharacterAnimLayerInstance::UpdateStanceTransitionAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
    UCharacterAnimInstance* const Base = GetBaseAnim();
    if (!Base) return;
    if (Base->_stanceTransitionRequested)
    {
        SetupStanceTransitionAnims(Context, Node);
        return;
    }
    if (!Base->_stanceTransitionPlaying) return;
    EAnimNodeReferenceConversionResult Result;
    const auto Player = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);
    if (Result != EAnimNodeReferenceConversionResult::Succeeded) return;
    Base->_stanceTransitionEvaluated = true;
    const UAnimSequenceBase* Clip = USequencePlayerLibrary::GetSequencePure(Player);
    if (!Clip || USequencePlayerLibrary::GetAccumulatedTime(Player) >= Clip->GetPlayLength() - KINDA_SMALL_NUMBER)
        Base->_stanceTransitionComplete = true;
}

void UCharacterAnimLayerInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    SetBaseAnim();
    auto* Diver = Cast<AHellDiver>(GetOwningActor());
    auto* State = Diver ? Diver->GetStateComponent() : nullptr;
    _actionAimAlpha = State && (State->IsMotionChanging() || State->IsWeaponChanging()) ? 0.f : 1.f;
    _predictedStopDistance = 0.f;
    _predictedPivotDistance = 0.f;
    const ACharacter* character = Cast<ACharacter>(GetOwningActor());
    const UCharacterMovementComponent* movement = character ? character->GetCharacterMovement() : nullptr;
    if (_baseAnimInst && movement)
    {
        const FVector velocity = movement->Velocity * FVector(1.f, 1.f, 0.f);
        const FVector acceleration = movement->GetCurrentAcceleration() * FVector(1.f, 1.f, 0.f);
        _predictedStopDistance = UAnimCharacterMovementLibrary::PredictGroundMovementStopLocation(
            velocity, movement->bUseSeparateBrakingFriction, movement->BrakingFriction,
            movement->GroundFriction, movement->BrakingFrictionFactor, movement->BrakingDecelerationWalking).Size2D();
        _predictedPivotDistance = UAnimCharacterMovementLibrary::PredictGroundMovementPivotLocation(
            acceleration, velocity, movement->GroundFriction).Size2D();
        if (!FMath::IsFinite(_predictedStopDistance)) _predictedStopDistance = 0.f;
        if (!FMath::IsFinite(_predictedPivotDistance)) _predictedPivotDistance = 0.f;
    }
}

UAnimSequenceBase* UCharacterAnimLayerInstance::SelectIdleAnimation(UCharacterAnimInstance* Base)
{
    if (!Base) return nullptr;
    switch (Base->GetCurrentState())
    {
    case ECharacterState::Standing:
    case ECharacterState::Sprinting: return _idleAnim;
    case ECharacterState::Crouching: return _crouchIdleAnim;
    case ECharacterState::Proning: return _proneIdleAnims.GetAnim(GetProneDirection());
    default: return nullptr;
    }
}

UAnimSequenceBase* UCharacterAnimLayerInstance::SelectStopAnimation(UCharacterAnimInstance* Base)
{
    if (!Base) return nullptr;
    UAnimSequenceBase* anim = nullptr;
    const auto direction = Base->GetVelocityLocomotionDirection();
    switch (Base->GetCurrentState())
    {
    case ECharacterState::Standing: anim = _walkStopAnims.GetAnim(direction); break;
    case ECharacterState::Sprinting: anim = _jogStopAnims.GetAnim(direction); break;
    case ECharacterState::Crouching: anim = _crouchStopAnims.GetAnim(direction); break;
    case ECharacterState::Proning: anim = _proneStopAnims.GetAnim(direction); break;
    default: break;
    }
    return anim ? anim : SelectIdleAnimation(Base);
}

bool UCharacterAnimLayerInstance::CanUseDistanceMatching(const UAnimSequenceBase* Animation, FName CurveName)
{
    const UAnimSequence* sequence = Cast<UAnimSequence>(Animation);
    return sequence && sequence->HasCurveData(CurveName, false) && sequence->IsCurveCompressedDataValid()
        && Cast<UAnimCurveCompressionCodec_UniformIndexable>(sequence->CompressedData.CurveCompressionCodec) != nullptr;
}

void UCharacterAnimLayerInstance::AdvanceDistanceOrTime(
    const FAnimUpdateContext& Context, const FSequenceEvaluatorReference& Evaluator, float Distance)
{
    if (!Context.GetContext()) return;
    UAnimSequenceBase* sequence = USequenceEvaluatorLibrary::GetSequence(Evaluator);
    if (!sequence) return;
    if (CanUseDistanceMatching(sequence, _distanceCurveName))
        UAnimDistanceMatchingLibrary::AdvanceTimeByDistanceMatching(Context, Evaluator,
            FMath::IsFinite(Distance) ? FMath::Max(0.f, Distance) : 0.f, _distanceCurveName);
    else
        USequenceEvaluatorLibrary::AdvanceTime(Context, Evaluator, 1.f);
}

void UCharacterAnimLayerInstance::MatchDistanceOrTime(
    const FAnimUpdateContext& Context, const FSequenceEvaluatorReference& Evaluator, float Distance)
{
    if (!Context.GetContext()) return;
    UAnimSequenceBase* sequence = USequenceEvaluatorLibrary::GetSequence(Evaluator);
    if (!sequence) return;
    if (FMath::IsFinite(Distance) && CanUseDistanceMatching(sequence, _distanceCurveName))
        UAnimDistanceMatchingLibrary::DistanceMatchToTarget(Evaluator, FMath::Max(0.f, Distance), _distanceCurveName);
    else
        USequenceEvaluatorLibrary::AdvanceTime(Context, Evaluator, 1.f);
}
