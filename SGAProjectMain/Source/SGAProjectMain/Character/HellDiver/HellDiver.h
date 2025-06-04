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

	void EquipGrenade();
	void EquipStratagem();
	void OnThrowReleased();
	void StartThrowPreview();
	void StopThrowPreview();
	UFUNCTION()
	void UpdateThrowSpline();
	class AThrowable* GetHeldThrowable() { return _heldThrowable; }

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

	void RefillAllItem();
	void RefillMag();

	void MotionChangeFinish();

	virtual void Landed(const FHitResult& Hit) override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
	class UHellDiverStateComponent* _stateComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", meta = (AllowPrivateAccess = "true"))
	class UHellDiverStatComponent* _statComponent;

	// 투척물
	UPROPERTY()
	class AThrowable* _heldThrowable = nullptr;
	UPROPERTY(EditDefaultsOnly, Category = "Game/Throwables", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ATimedGrenadeBase> _grenadeClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Throwables")
	ATimedGrenadeBase* _equippedGrenade;

	UPROPERTY(EditDefaultsOnly, Category = "Game/Throwables", meta = (AllowPrivateAccess = "true"))
	class UStratagemComponent* _stratagemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Throwables")
	AStratagem* _equippedStratagem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Throwables/Trajectory", meta = (AllowPrivateAccess = "true"))
	class USplineComponent* _trajectorySpline;

	FTimerHandle _throwPreviewTimer;

	bool _isPreviewingThrow = false;

	UPROPERTY(EditDefaultsOnly, Category = "Game/Throwables/Trajectory")
	UStaticMesh* _trajectoryMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Game/Throwables/Trajectory")
	UMaterialInterface* _trajectoryMaterial;

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
};
