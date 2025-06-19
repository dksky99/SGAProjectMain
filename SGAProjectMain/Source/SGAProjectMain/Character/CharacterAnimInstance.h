// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UCharacterAnimInstance();

	virtual void NativeInitializeAnimation() override;


	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION()
	void PlayAnimMontage(UAnimMontage* animMontage);


	void JumpToSection(int32 sectionIndex);

	bool IsMoving();

protected:

	class ACharacterBase* _pawn=nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true"))
	float _speed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true"))
	bool _isFalling;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimMontage", meta = (AllowPrivateAccess = "true"))
	float _vertical;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimMontage", meta = (AllowPrivateAccess = "true"))
	float _horizontal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimOffset", meta = (AllowPrivateAccess = "true"))
	float _yaw = 0; // 좌우 회전

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimOffset", meta = (AllowPrivateAccess = "true"))
	float _pitch = 0; // 상하 회전


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimOffset", meta = (AllowPrivateAccess = "true"))
	float _deltaAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimOffset", meta = (AllowPrivateAccess = "true"))
	bool _isTurnLeft = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimOffset", meta = (AllowPrivateAccess = "true"))
	bool _isTurnRight = false;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimOffset", meta = (AllowPrivateAccess = "true"))
	bool _isMoving = false;




};
