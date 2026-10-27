// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"

#include "CharacterAnimInstance.h"
#include "CharacterAnimLayerInstance.generated.h"

USTRUCT(BlueprintType)
struct FDirectionalAnimations
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations") 
	UAnimSequenceBase* Forward;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations") 
	UAnimSequenceBase* Backward;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations") 
	UAnimSequenceBase* Right;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations") 
	UAnimSequenceBase* Left;

public:
	
	UAnimSequenceBase* GetAnim(int32 n)
	{
		switch (n)
		{
		case 0:
			return Forward;
			break;
		case 1:
			return Backward;
			break;
		case 2:
			return Right;
			break;
		case 3:
			return Left;
			break;
		default:
			return nullptr;
			break;
		}
	}
	UAnimSequenceBase* GetAnim(ELocomotionDirections dir)
	{
		return GetAnim((int32)dir);
	}

};
/**
 *  크라우치는 플레이어는 기본으로 갖지만 적 유닛들은 쓰거나 안씀. 쓰는경우는 부상으로 다리를 절고있을때. 다리 파손시 혹은 부상으로 걷기보다 느려질경우 사용할 예정
 *  기본은 walk 
 *  광폭이나 여러 방식으로 가속한다면 jog
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UCharacterAnimLayerInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	//
	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Idle",  meta = (BlueprintThreadSafe))
	void IdleOnUpdate(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Cycle", meta = (BlueprintThreadSafe))
	void CycleOnUpdate(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);


	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Idle", meta = (BlueprintThreadSafe))
	ELocomotionDirections GetProneDirection();

	//움직임 멈춤.
	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Stop", meta = (BlueprintThreadSafe))
	void SetupStopAnims(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Stop", meta = (BlueprintThreadSafe))
	void UpdateStopAnims(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	//움직임 시작 
	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Start", meta = (BlueprintThreadSafe))
	void SetupStartAnims(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Start", meta = (BlueprintThreadSafe))
	void UpdateStartAnims(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);


	//역방향으로 갑작스런 방향전환
	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Pivot", meta = (BlueprintThreadSafe))
	class UAnimSequenceBase* SelectedPivotAnim();
	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Pivot", meta = (BlueprintThreadSafe))
	void SetupPivotAnims(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Pivot", meta = (BlueprintThreadSafe))
	void UpdatePivotAnims(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);


	//제자리에서 회전
	UFUNCTION(BlueprintCallable, Category = "Game/Animation/TurnInPlace", meta = (BlueprintThreadSafe))
	void SetupTurnInPlaceEntryState(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "Game/Animation/TurnInPlace", meta = (BlueprintThreadSafe))
	void SetupTurnInPlaceEntryAnims(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "Game/Animation/TurnInPlace", meta = (BlueprintThreadSafe))
	UAnimSequenceBase* SelectTurnInPlaceAnimation();
	UFUNCTION(BlueprintCallable, Category = "Game/Animation/TurnInPlace", meta = (BlueprintThreadSafe))
	void UpdateTurnInPlaceEntryAnims(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	//점프 애니메이션.
	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Jump", meta = (BlueprintThreadSafe))
	void SetupJumpFallLandAnim(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Jump", meta = (BlueprintThreadSafe))
	void UpdateJumpFallLandAnim(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Jump", meta = (BlueprintThreadSafe))
	void SetupDodgeStartAnim(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Jump", meta = (BlueprintThreadSafe))
	void UpdateDodgeStartAnim(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Jump", meta = (BlueprintThreadSafe))
	void SetupDodgeLoopAnim(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Jump", meta = (BlueprintThreadSafe))
	void UpdateDodgeLoopAnim(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Jump", meta = (BlueprintThreadSafe))
	void SetupDodgeEndAnim(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Jump", meta = (BlueprintThreadSafe))
	void UpdateDodgeEndAnim(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);


	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Transition", meta = (BlueprintThreadSafe))
	void SetupStanceTransitionAnims(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);
	
protected:
	virtual void NativeInitializeAnimation() override;

	void SetBaseAnim();

	class UCharacterAnimInstance* GetBaseAnim();


protected:
	UPROPERTY(Transient,BlueprintReadOnly, Category = "Cache")
	class UCharacterAnimInstance* _baseAnimInst;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Idle", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _idleAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Cycle", meta = (AllowPrivateAccess = "true"))
	FDirectionalAnimations _walkCycleAnims;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Cycle", meta = (AllowPrivateAccess = "true"))
	FDirectionalAnimations _jogCycleAnims;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Stop", meta = (AllowPrivateAccess = "true"))
	FDirectionalAnimations _walkStopAnims;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Stop", meta = (AllowPrivateAccess = "true"))
	FDirectionalAnimations _jogStopAnims;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Start", meta = (AllowPrivateAccess = "true"))
	FDirectionalAnimations _walkStartAnims;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Start", meta = (AllowPrivateAccess = "true"))
	FDirectionalAnimations _jogStartAnims;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Pivot", meta = (AllowPrivateAccess = "true"))
	FDirectionalAnimations _walkPivotAnims;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Pivot", meta = (AllowPrivateAccess = "true"))
	FDirectionalAnimations _jogPivotAnims;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Animation/TurnInPlace", meta = (AllowPrivateAccess = "true"))
	bool _bShouldTurnLeft = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/TurnInPlace", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _turnRight90;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/TurnInPlace", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _turnRight180;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/TurnInPlace", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _turnLeft90;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/TurnInPlace", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _turnLeft180;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Animation/TurnInPlace", meta = (AllowPrivateAccess = "true"))
	float _turnInPlaceTime = 0.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Animation/TurnInPlace", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _finalTurnAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Crouch", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _crouchIdleAnim;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Crouch", meta = (AllowPrivateAccess = "true"))
	FDirectionalAnimations _crouchStartAnims;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Crouch", meta = (AllowPrivateAccess = "true"))
	FDirectionalAnimations _crouchCycleAnims;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Crouch", meta = (AllowPrivateAccess = "true"))
	FDirectionalAnimations _crouchStopAnims;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Crouch", meta = (AllowPrivateAccess = "true"))
	FDirectionalAnimations _crouchPivotAnims;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Crouch", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _crouchTurnRight90;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Crouch", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _crouchTurnRight180;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Crouch", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _crouchTurnLeft90;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Crouch", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _crouchTurnLeft180;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Prone", meta = (AllowPrivateAccess = "true"))
	FDirectionalAnimations _proneStartAnims;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Prone", meta = (AllowPrivateAccess = "true"))
	FDirectionalAnimations _proneIdleAnims;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Prone", meta = (AllowPrivateAccess = "true"))
	FDirectionalAnimations _proneCycleAnims;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Prone", meta = (AllowPrivateAccess = "true"))
	FDirectionalAnimations _proneStopAnims;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Prone", meta = (AllowPrivateAccess = "true"))
	FDirectionalAnimations _pronePivotAnims;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Prone", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _proneTurnRight90;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Prone", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _proneTurnRight180;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Prone", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _proneTurnLeft90;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Prone", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _proneTurnLeft180;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Prone", meta = (AllowPrivateAccess = "true"))
	ELocomotionDirections _proneDirection;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Prone", meta = (AllowPrivateAccess = "true"))
	FRotator _proneRotation;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Jump", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _jumpApex;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Jump", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _jumpFallLoop;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Jump", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _jumpFallLand;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Jump", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _jumpFallLandAdditiveRecovery;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Dodge", meta = (AllowPrivateAccess = "true"))
	FDirectionalAnimations _dodgeStartAnims;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Dodge", meta = (AllowPrivateAccess = "true"))
	FDirectionalAnimations _dodgeLoopAnims;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Dodge", meta = (AllowPrivateAccess = "true"))
	FDirectionalAnimations _dodgeEndAnims;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Dodge", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _finaldodgeAnim;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/AO", meta = (AllowPrivateAccess = "true"))
	UBlendSpace* _aoStandUp;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/AO", meta = (AllowPrivateAccess = "true"))
	UBlendSpace* _aoCrouch;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/AO", meta = (AllowPrivateAccess = "true"))
	UBlendSpace* _aoProneFwd;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/AO", meta = (AllowPrivateAccess = "true"))
	UBlendSpace* _aoProneBwd;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/AO", meta = (AllowPrivateAccess = "true"))
	UBlendSpace* _aoProneLeft;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/AO", meta = (AllowPrivateAccess = "true"))
	UBlendSpace* _aoProneRight;



	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Transition", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _standToCrouchAnim;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Transition", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _crouchToStandAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Transition", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _standToProneAnim;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Transition", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _proneToStandAnim;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Transition", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _proneToCrouchAnim;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Transition", meta = (AllowPrivateAccess = "true"))
	UAnimSequenceBase* _crouchToProneAnim;



	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game/Animation/Transition", meta = (AllowPrivateAccess = "true"))
	FName _distanceCurveName = TEXT("Distance");
};
