// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"

#include "CharacterBase.h"
#include "PlayerCharacter.h"

#include "GameFramework/PawnMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Math/UnrealMathUtility.h"

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

			_vertical = _pawn->MyVertical();
			_horizontal = _pawn->MyHorizontal();
			_deltaAngle = _pawn->MyDeltaAngle();

			FRotator controlRotation = _pawn->GetControlRotation();
			FRotator actorRotation = _pawn->GetActorRotation();
			_yaw = FMath::FindDeltaAngleDegrees(actorRotation.Yaw, controlRotation.Yaw);
			_pitch = FMath::FindDeltaAngleDegrees(actorRotation.Pitch, controlRotation.Pitch);

			auto player = Cast<APlayerCharacter>(_pawn);
			if (player)
			{
				_isTurnLeft = player->_isTurnLeft;
				_isTurnRight = player->_isTurnRight;
			}
		}
	}
}

void UCharacterAnimInstance::PlayAnimMontage(UAnimMontage* animMontage)
{
	if (!Montage_IsPlaying(animMontage))
	{
		Montage_Play(animMontage);
	}
}


void UCharacterAnimInstance::JumpToSection(int32 sectionIndex)
{
	FName sectionName = FName(*FString::Printf(TEXT("Section%d"), sectionIndex));
	Montage_JumpToSection(sectionName);
}

bool UCharacterAnimInstance::IsMoving()
{
	
	return 0.1<=(_vertical * _vertical + _horizontal + _horizontal);
}
