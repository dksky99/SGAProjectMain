// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"

#include "CharacterBase.h"
#include "PlayerCharacter.h"
#include "CharacterStateComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Math/UnrealMathUtility.h"

#include "KismetAnimationLibrary.h"

UCharacterAnimInstance::UCharacterAnimInstance()
{
}

void UCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	auto pawn = TryGetPawnOwner();
	if (pawn)
	{
		_pawn = Cast<ACharacterBase>(pawn);
	}
	else
		_pawn = nullptr;


}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (_pawn)
	{
		if (_pawn != nullptr)
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
	}
}

void UCharacterAnimInstance::PlayAnimMontage(UAnimMontage* animMontage, float playRate)
{
	if (!Montage_IsPlaying(animMontage))
	{
		Montage_Play(animMontage,playRate);
	}
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
