// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../CharacterBase.h"
#include "HellDiverStateComponent.h"
#include "HellDiver.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FGrenadeChanged, int32, int32);

UCLASS()
class SGAPROJECTMAIN_API AHellDiver : public ACharacterBase
{
	GENERATED_BODY()
public:

	AHellDiver(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	class UHellDiverStateComponent* GetStateComponent();
	class UHellDiverStatComponent* GetStatComponent();
	class UMotionWarpingComponent* GetMotionWarp() const;
	class UHellDiverInvenComponent* GetInvenComponent();

	// 투척물
	void EquipGrenade();
	void EquipStratagem();
	void OnThrowReleased(class UAnimMontage* Montage, bool bInterrupted);
	void StartThrowPreview();
	void StopThrowPreview();
	UFUNCTION()
	void UpdateThrowSpline();
	class AThrowable* GetHeldThrowable() { return _heldThrowable; }

	virtual bool AttackMelee() override;
	// 각성제
	void UseStimPack();

	void StartSprint();
	void FinishSprint();

	void StartCrouch();
	void FinishCrouch();


	void StartProne();
	void FinishProne();

	void Rolling();
	void FinishRolling();

	void Standing();
	void Sprinting();
	void Crouching();
	void Proning();

	// 총
	virtual void InitWeapon(); // 처음 한 번만 실행
	void EquipGun(int32 index); // 슬롯에서 인덱스로 장착
	virtual void PickupGun(class AGunBase* gun); // 총을 직접 습득
	void SwitchGun(int32 index);
	AGunBase* GetEquippedGun();

	void UnequipGun();	//4번키나 스트라타젬을 입력 완료했을때 손에 쥐었던 장비를 기존의 슬룻에 전부 돌려놓는다.
	
	void SaveLastState(int32 index);
	void BackupLastState();

	void StartFiring();
	void StopFiring();
	void StartAiming();
	void StopAiming();

	void Reload();
	bool CanReloadUsingBackpack(); // gun에서 reload 시 호출

	void EquipBackpack(class ABackpack* backpack);
	void UseBackpack(int32 amount = 1);
	virtual void AddSample(struct FSampleBundle sample);

	// 보급
	void RefillAllItem();
	void RefillSpare();
	void RefillGrenade();
	void RefillStimPack();

	void StratagemInputting();

	void MotionChangeFinish();

	virtual void Landed(const FHitResult& Hit) override;

	void SoftLanding();
	void HardLanding();

	void FinishLanding(class UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(BlueprintCallable)
	virtual FRotator Focusing();

	void Throwing();

	//virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	FTransform GetLeftHandSocketTransform() const;
	FTransform GetMuzzleTransform() const;
	FVector GetJointTargetLocation() { return _jointTargetLoc; }

	FGrenadeChanged _grenadeChanged;

	virtual void KnockDown(float time=3.0f) override;
	virtual void RecoverFromKnockDown() override;
	virtual void Dead() override;
	virtual void AfterDead();
	virtual void RecoverFromDead() override;


	virtual void StrongStagger(float time) override;

protected:

	virtual void PossessedBy(AController* NewController) override;

	virtual void UnPossessed() override;


	FTransform GetHandSocketTransform() const;
	FTransform GetEquip1SocketTransform() const;
	FTransform GetEquip2SocketTransform() const;
	FTransform GetEquip3SocketTransform() const;



	UFUNCTION()
	void SetCollisionState(ECharacterState newState);
	void SetCollisionCamera(class UCollisionCameraDataAsset data);
	virtual void SetStandingCollisionCamera();
	virtual void SetCrouchingCollisionCamera();
	virtual void SetProningCollisionCamera() ;

	// 플레이어에서 위젯 관리 위치 (순서 보장)
	virtual void OnPreSwitchGun(class AGunBase* prevGun) {};
	virtual void OnPostSwitchGun(class AGunBase* newGun) {};

private:
	void ClearThrowSpline(); 
	void DrawThrowSplineMeshes();

protected: 

	FTimerHandle _rollingTimerHandle;

	// 상태
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/HellDiver/State", meta = (AllowPrivateAccess = "true"))
	class UHellDiverStateComponent* _stateComponent;
	
	UPROPERTY()
	class UHellDiverStatComponent* _statComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/HellDiver/Pakour", meta = (AllowPrivateAccess = "true"))
	class UPakourComponent* _pakourComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/HellDiver/akour", meta = (AllowPrivateAccess = "true"))
	class UMotionWarpingComponent* _motionWarpComponent;

	// 스프린트 관리
	FTimerHandle _sprintCooldownHandle;
	bool _isSprintCoolTime = false;

	// 인벤토리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Inventory", meta = (AllowPrivateAccess = "true"))
	class UHellDiverInvenComponent* _invenComponent;

	// 투척물
	UPROPERTY()
	class AThrowable* _heldThrowable = nullptr;
	UPROPERTY(EditDefaultsOnly, Category = "Game/Throwables", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ATimedGrenadeBase> _grenadeClass;

	UPROPERTY(EditDefaultsOnly, Category = "Game/Throwables")
	int32 _maxGrenade = 4;

	UPROPERTY(VisibleAnywhere, Category = "Game/Throwables")
	int32 _curGrenade = 2;

	UPROPERTY(EditDefaultsOnly, Category = "Game/Stratagem", meta = (AllowPrivateAccess = "true"))
	class UStratagemComponent* _stratagemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Throwables/Trajectory", meta = (AllowPrivateAccess = "true"))
	class USplineComponent* _trajectorySpline;

	FTimerHandle _throwPreviewTimer;

	bool _isPreviewingThrow = false;

	UPROPERTY(EditDefaultsOnly, Category = "Game/Throwables/Trajectory")
	UStaticMesh* _trajectoryMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Game/Throwables/Trajectory")
	UMaterialInterface* _trajectoryMaterial;

	//각성제
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/StimPack")
	class UStimPackComponent* _stimPackComponent;
	

	int32 _lastState = 0;

	// 생성된 메쉬 저장용
	UPROPERTY()
	TArray<class USplineMeshComponent*> _trajectoryMeshPool;

	// 자세
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stance", meta = (AllowPrivateAccess = "true"))
	class UCollisionCameraDataAsset* _standingStance;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stance", meta=(AllowPrivateAccess = "true"))
	class UCollisionCameraDataAsset* _crouchingStance;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stance", meta=(AllowPrivateAccess = "true"))
	class UCollisionCameraDataAsset* _proningStance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Anim")
	FVector _jointTargetLoc = FVector(20,45,-90);


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* _softLandingMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* _hardLandingMontage;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* _throwingMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* _stratagemInputMontage;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	class UUnitAttackDataAsset* _oneHandedMelee;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	class UUnitAttackDataAsset* _twoHandedMelee;
};
