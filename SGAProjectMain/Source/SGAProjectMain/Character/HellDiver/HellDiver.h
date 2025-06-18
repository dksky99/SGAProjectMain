// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../CharacterBase.h"
#include "HellDiverStateComponent.h"
#include "HellDiver.generated.h"



UCLASS()
class SGAPROJECTMAIN_API AHellDiver : public ACharacterBase
{
	GENERATED_BODY()
public:

	AHellDiver(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	class UHellDiverStateComponent* GetStateComponent();
	class UHellDiverStatComponent* GetStatComponent();

	class UMotionWarpingComponent* GetMotionWarp() const;
	// 투척물
	void EquipGrenade();
	void EquipStratagem();
	void OnThrowReleased();
	void StartThrowPreview();
	void StopThrowPreview();
	UFUNCTION()
	void UpdateThrowSpline();
	class AThrowable* GetHeldThrowable() { return _heldThrowable; }

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

	class AGunBase* SpawnGun(TSubclassOf<AGunBase> gunClass);
	void EquipGun(AGunBase* gun);
	AGunBase* GetEquippedGun() { return _equippedGun; }

	// 보급
	void RefillAllItem();
	void RefillMag();
	void RefillGrenade();
	void RefillStimPack();


	void MotionChangeFinish();

	virtual void Landed(const FHitResult& Hit) override;

	UFUNCTION(BlueprintCallable)
	virtual FRotator Focusing();

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	FTransform GetLeftHandSocketTransform() const;
	FVector GetJointTargetLocation() { return _jointTargetLoc; }
protected:

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

private:
	void ClearThrowSpline(); 
	void DrawThrowSplineMeshes();

protected: 

	FTimerHandle _rollingTimerHandle;

	// 상태
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/State", meta = (AllowPrivateAccess = "true"))
	class UHellDiverStateComponent* _stateComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stat", meta = (AllowPrivateAccess = "true"))
	class UHellDiverStatComponent* _statComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Pakour", meta = (AllowPrivateAccess = "true"))
	class UPakourComponent* _pakourComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Pakour", meta = (AllowPrivateAccess = "true"))
	class UMotionWarpingComponent* _motionWarpComponent;

	// 투척물
	UPROPERTY()
	class AThrowable* _heldThrowable = nullptr;
	UPROPERTY(EditDefaultsOnly, Category = "Game/Throwables", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ATimedGrenadeBase> _grenadeClass;

	UPROPERTY(EditDefaultsOnly, Category = "Game/Throwables")
	int32 _maxGrenade = 4;

	UPROPERTY(VisibleAnywhere, Category = "Game/Throwables")
	int32 _curGrenade = 2;

	UPROPERTY(EditDefaultsOnly, Category = "Game/Throwables", meta = (AllowPrivateAccess = "true"))
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

	// 총기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Gun", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AGunBase> _gunClass1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Gun", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AGunBase> _gunClass2;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Gun", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AGunBase> _gunClass3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Gun")
	AGunBase* _equippedGun;

	UPROPERTY()
	TArray<AGunBase*> _gunSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Anim")
	FVector _jointTargetLoc = FVector(20,45,-90);

};
