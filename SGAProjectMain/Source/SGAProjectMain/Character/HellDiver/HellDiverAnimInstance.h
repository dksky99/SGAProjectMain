// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../CharacterAnimInstance.h"
#include "HellDiverStateComponent.h"
#include "../../Interface/HelldiverAnimInterface.h"
#include "HellDiverAnimInstance.generated.h"

/*
	믹사모에서 추가해올 애니메이션
	Taunt : 일부를 가져와서 스트라타젬을 누르는 애니메이션으로 사용할 수 있을것같다
	Flying Death : 일부를 가져와서 뒤로 Rolling할때 사용할 수 있을것같다
	GoalKeeper Diving Safe : 일부를 가져와서 옆으로 Rolling할때 사용할 수 있을것 같다.


*/


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMoveAnimStateChanged, FString, newState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLookAnimStateChanged, FString, newState);

UCLASS()
class SGAPROJECTMAIN_API UHellDiverAnimInstance : public UCharacterAnimInstance,public IHelldiverAnimInterface
{
	GENERATED_BODY()
	friend class FHelldiverAnimEquipChangeTest;
public:
	UHellDiverAnimInstance();

	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    // Game-thread snapshots consumed by the final component-space aiming node.
    UPROPERTY(BlueprintReadOnly, Category="Game/Animation/Aim") float _upperBodyAimAlpha = 0.f;
    UPROPERTY(BlueprintReadOnly, Category="Game/Animation/Aim") FVector _aimBodyForwardMesh = FVector(0,1,0);
    UPROPERTY(BlueprintReadOnly, Category="Game/Animation/Aim") FVector _aimBodyUpMesh = FVector::UpVector;
    UPROPERTY(BlueprintReadOnly, Category="Game/Animation/Aim") FTransform _upperBodyLeftGripInHand = FTransform::Identity;
    UPROPERTY(BlueprintReadOnly, Category="Game/Animation/Aim") float _upperBodyLeftHandAlpha = 0.f;


	// Value-only getters for AnimGraph; all UObject/socket reads happen on the game thread.
	UFUNCTION(BlueprintPure, Category = "Game/Anim/HandIK", meta = (BlueprintThreadSafe))
	FVector GetLeftHandIKTargetWorld() const { return _leftHandIKTargetWorld; }

	UFUNCTION(BlueprintPure, Category = "Game/Anim/HandIK", meta = (BlueprintThreadSafe))
	float GetLeftHandIKAlpha() const { return _leftHandIKAlpha; }

	UFUNCTION(BlueprintPure, Category = "Animation|Aim", meta = (BlueprintThreadSafe))
	float GetAimRigAlpha() const { return _aimRigAlpha; }

	UFUNCTION(BlueprintPure, Category = "Game/Anim/HandIK", meta = (BlueprintThreadSafe))
	bool HasValidLeftHandIKTarget() const { return _hasValidLeftHandIKTarget; }

	//현재 애니메이션 변환 상태를 확인.
	void GetCurrentMoveNode();

	UFUNCTION()
	void AnimNotify_Reload();


	UFUNCTION(BlueprintCallable)
	void AnimNotify_FootStep();


	FOnMoveAnimStateChanged _moveChanged;
	FOnLookAnimStateChanged _lookChanged;

	virtual void SetWeaponOffset(FTransform offset, bool useAim=true) override;

protected:
	bool MoveStateChanged(FString curState);
	bool LookStateChanged(FString curState);

	void AimFocus(float DeltaSeconds);
	bool IsStableState_Move();
	bool IsStableState_Look();
	bool IsUsingLeftHand();
	bool IsUsingFocusing();
	void CheckEquipChange(uint8 index);

	void GetAimOffset(float deltaTime);
	void CalcYaw();
	void CalcPitch();
	void CalcAimPitch(float deltaTime);
	void CalcAimYaw(float deltaTime);

	void DrawDebugs();
	void ResetRuntimeSnapshot();
	void UpdateLeftHandIKSnapshot();
	bool _refreshWeaponOffsetOnNextUpdate = false;
	bool _weaponOffsetExplicitlyDisabled = false;

protected:
	UPROPERTY(Transient)
	class AHellDiver* _hellDiver = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	ECharacterState _characterState = ECharacterState::Standing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	EActionState _actionState = EActionState::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	EWeaponType _weaponState = EWeaponType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	ELifeState _lifeState = ELifeState::Alive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FRotator _focusRotate=FRotator();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	class AGunBase* _curWeapon = nullptr;


	//알파가 1이면 조준 0이면 평범한 애니메이션
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float _focusAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float _focusSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool _useLeftHand = false;

	// World-space IK_left target. Never substitute the hand_r-local LeftGrip transform.
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Game/Anim/HandIK")
	FVector _leftHandIKTargetWorld = FVector::ZeroVector;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Game/Anim/HandIK")
	float _leftHandIKAlpha = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Animation|Aim")
	float _aimRigAlpha = 0.f;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Game/Anim/HandIK")
	bool _hasValidLeftHandIKTarget = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool _changeWeapon = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FTransform _leftHandTrans = FTransform();
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FTransform _muzzleTrans = FTransform();


	//총구의 위치->총 메쉬의 루트로부터=오른손 소켓으로부터-> 오른손소캣트랜스폼에 이걸 곱하면 총구의 트랜스폼을 얻을수있다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FTransform _muzzleTrans_Relation = FTransform();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FTransform _rightHand_WorldTrans = FTransform();

	//조준보정 강도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float _aimAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float _angleLimit = 130.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float _interpSpeed = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FVector _jointTargetLoc = FVector();



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	uint8 _curEquipIndex=0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool _useLeftHandIk = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float _moveSpeed = 0.f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool _useFocusing = false;
	float _rotateSpeed = 350;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool _isAiming = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool _isFocusing = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool _isFiring = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool _isReloading = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool _isRolling = false;




	float _prevActorYaw = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float _addYaw=0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float _addPitch = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float _finalYaw = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float _finalPitch = 0.0f;

	FString _currentMoveState;
	FString _currentLookState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FTransform _weaponOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool _bUseAim = false;

	// IHelldiverAnimInterface을(를) 통해 상속됨
	bool SetLeftHandLocation(FVector inHandLocation) override;

};
