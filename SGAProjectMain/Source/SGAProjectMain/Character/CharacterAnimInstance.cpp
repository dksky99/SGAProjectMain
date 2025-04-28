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

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	auto pawn = TryGetPawnOwner();
	if (pawn)
	{
		ACharacterBase* character = Cast<ACharacterBase>(pawn);
		if (character != nullptr)
		{
			_speed = character->GetVelocity().Size();
			_isFalling = character->GetMovementComponent()->IsFalling();

			_vertical = character->MyVertical();
			_horizontal = character->MyHorizontal();

			FRotator controlRotation = character->GetControlRotation();
			FRotator actorRotation = character->GetActorRotation();
			_yaw = FMath::FindDeltaAngleDegrees(actorRotation.Yaw, controlRotation.Yaw);
			_pitch = FMath::FindDeltaAngleDegrees(actorRotation.Pitch, controlRotation.Pitch);

			auto player = Cast<APlayerCharacter>(character);
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
