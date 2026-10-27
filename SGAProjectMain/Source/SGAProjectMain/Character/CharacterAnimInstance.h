// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "../Interface/AnimInstanceInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "CharacterStateComponent.h"
#include "CharacterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class ELocomotionDirections : uint8
{
	Forward,
	Backward,
	Right,
	Left
};

UENUM(BlueprintType)
enum class ERootYawOffsetMode : uint8
{
	Accumulate,
	BlendOut,
	Hold
};
class UCharacterAnimLayerInstance;
/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMotionFinished);
UCLASS()
class SGAPROJECTMAIN_API UCharacterAnimInstance : public UAnimInstance,public IAnimInstanceInterface
{
	GENERATED_BODY()
	friend class UCharacterAnimLayerInstance;
public:
	UCharacterAnimInstance();

	virtual void NativeInitializeAnimation() override;


	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;


	UFUNCTION()
	float PlayAnimMontage(UAnimMontage* animMontage,float playRate=1.0f);


	void JumpToSection(int32 sectionIndex);

	bool IsMoving();

	UFUNCTION()
	void AnimNotify_Attack();


	// IAnimInstanceInterface을(를) 통해 상속됨
	void RecieveGroundDistance(float dist)  override;


	void RecieveCharacterStateChange(int32 n)  override;

	ECharacterState GetCurrentState() { return _currentState; }
	ELocomotionDirections GetVelocityLocomotionDirection() { return _velocityLocomotionDirection; }
	ELocomotionDirections GetAccelerationLocomotionDirection() { return _accelerationLocomotionDirection; }
	ERootYawOffsetMode GetRootYawOffsetMode() { return _rootYawOffsetMode; }
	
	class UCharacterMovementComponent* GetCharacterMovement();

	FOnMotionFinished _motionFinished;
protected:
	class ACharacterBase* TryGetCBOwner();

	FTransform GetAimBone();

	void GetVelocityData();
	void GetAccelerationData();
	void GetLocationData();
	void GetRotationData(float deltaTime);
	void UpdateOrientationData();
	ELocomotionDirections CalculateLocomotionDirection(float currentLocomotionAngle,ELocomotionDirections currentDirection,float backwardMin=-130.f, float backwardMax=130.f, float forwardMin=-50.f, float forwardMax=50.f, float deadZone=20.f);
	virtual void GetCharacterState(float deltaTime);
	void UpdateRootYawOffset(float deltaTime);
	void SetRootYawOffset(float angle);
	

protected:

	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Idle", meta = (BlueprintThreadSafe))
	void UpdateIdleState(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "Game/Animation/RootYawOffset", meta = (BlueprintThreadSafe))
	void ProcessTurnYawCurve();

	UFUNCTION(BlueprintCallable, Category = "Game/Animation/PivotData", meta = (BlueprintThreadSafe))
	void SetupPivotState(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);


	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Jump", meta = (BlueprintThreadSafe))
	void SetupJumpApexPose(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);


	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Jump", meta = (BlueprintThreadSafe))
	void SetupStateTransition(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, Category = "Game/Animation/Jump", meta = (BlueprintThreadSafe))
	void FinishStateTransition(const struct FAnimUpdateContext& Context, const struct FAnimNodeReference& Node);
protected:

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	class ACharacterBase* _pawn = nullptr;

	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true"))
	float _speed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true"))
	bool _isFalling;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true"))
	float _moveRotation = 0.0f;

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


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool _isActing = false;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/VelocityData", meta = (AllowPrivateAccess = "true"))
	FVector _characterVelocity=FVector();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/VelocityData", meta = (AllowPrivateAccess = "true"))
	FVector _characterVelocity2D=FVector();


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/LocationData", meta = (AllowPrivateAccess = "true"))
	FVector _worldLocation = FVector();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/LocationData", meta = (AllowPrivateAccess = "true"))
	FVector _lastFrameWorldLocation = FVector();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/LocationData", meta = (AllowPrivateAccess = "true"))
	float _deltaLocation = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/Gate", meta = (AllowPrivateAccess = "true"))
	ECharacterState _currentState = ECharacterState::Standing;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/Gate", meta = (AllowPrivateAccess = "true"))
	ECharacterState _incommingState = ECharacterState::Standing;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/Gate", meta = (AllowPrivateAccess = "true"))
	ECharacterState _lastFrameState = ECharacterState::Standing;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/Gate", meta = (AllowPrivateAccess = "true"))
	bool _bIsStateChanged=false;



	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/RotationData", meta = (AllowPrivateAccess = "true"))
	FVector _targetPos_World;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	FVector _targetPos_Mesh = FVector(0.f, 400.f, 200.f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/RotationData", meta = (AllowPrivateAccess = "true"))
	FName _aimBoneName = TEXT("head_01");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/RotationData", meta = (AllowPrivateAccess = "true"))
	FRotator _worldRotation=FRotator();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/RotationData", meta = (AllowPrivateAccess = "true"))
	float _actorYaw = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/RotationData", meta = (AllowPrivateAccess = "true"))
	float _lastFrameActorYaw = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/RotationData", meta = (AllowPrivateAccess = "true"))
	float _deltaActorYaw = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/RotationData", meta = (AllowPrivateAccess = "true"))
	float _leanAngle = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/RotationData", meta = (AllowPrivateAccess = "true"))
	float _aimPitch = 0.f;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/LocomotionData", meta = (AllowPrivateAccess = "true"))
	ELocomotionDirections _velocityLocomotionDirection= ELocomotionDirections::Forward;;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/LocomotionData", meta = (AllowPrivateAccess = "true"))
	ELocomotionDirections _lastFrameVelocityLocomotionDirection= ELocomotionDirections::Forward;;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/LocomotionData", meta = (AllowPrivateAccess = "true"))
	ELocomotionDirections _accelerationLocomotionDirection=ELocomotionDirections::Forward;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/LocomotionData", meta = (AllowPrivateAccess = "true"))
	float _velocityLocomotionAngle=0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/LocomotionData", meta = (AllowPrivateAccess = "true"))
	float _accelerationLocomotionAngle=0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/LocomotionData", meta = (AllowPrivateAccess = "true"))
	float _velocityLocomotionAngleWithOffset=0.f;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/AccelerationData", meta = (AllowPrivateAccess = "true"))
	FVector _acceleration=FVector();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/AccelerationData", meta = (AllowPrivateAccess = "true"))
	FVector _acceleration2D = FVector();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/AccelerationData", meta = (AllowPrivateAccess = "true"))
	FVector _pivotAcceleration2D = FVector();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/AccelerationData", meta = (AllowPrivateAccess = "true"))
	bool _bIsAccelerating = false;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/RootYawOffset", meta = (AllowPrivateAccess = "true"))
	float _rootYawOffset=0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/RootYawOffset", meta = (AllowPrivateAccess = "true"))
	float _turnYawCurveValue = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/RootYawOffset", meta = (AllowPrivateAccess = "true"))
	float _lastFrameTurnYawCurveValue = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/RootYawOffset", meta = (AllowPrivateAccess = "true"))
	ERootYawOffsetMode _rootYawOffsetMode = ERootYawOffsetMode:: Accumulate;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/RootYawOffset", meta = (AllowPrivateAccess = "true"))
	FFloatSpringState _rootYawOffsetSpringState;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/Jump", meta = (AllowPrivateAccess = "true"))
	bool _bIsJumping = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/Jump", meta = (AllowPrivateAccess = "true"))
	bool _bIsFalling = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/Jump", meta = (AllowPrivateAccess = "true"))
	bool _bIsOnAir = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/Jump", meta = (AllowPrivateAccess = "true"))
	float _timeToJumpApex=0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/Jump", meta = (AllowPrivateAccess = "true"))
	float _groundDistance = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Anim/Jump", meta = (AllowPrivateAccess = "true"))
	float _timeFalling = 0.f;






};
