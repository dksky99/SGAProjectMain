// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SentryTurret.generated.h"

UCLASS()
class SGAPROJECTMAIN_API ASentryTurret : public AActor
{
	GENERATED_BODY()

public:
	ASentryTurret();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// -------------------------------
	// 외부 호출 함수(간단)
	// -------------------------------

	// 외부에서 강제로 타깃을 지정
	void SetTargetActor(AActor* target);

	UFUNCTION(BlueprintCallable, Category = "Game/Stratagem/Sentry")
	void AIStartFire();

	UFUNCTION(BlueprintCallable, Category = "Game/Stratagem/Sentry")
	void AIStopFire();

	// 데미지 수신
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	// Perception이 참조할 시야 원점/방향을 머즐 기준으로 제공
	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;

	// 외부용 Getter
	FVector GetMuzzleLocation() const;
	FVector GetMuzzleForward() const;
	FTransform GetMeshTransform() const;

protected:
	// =========================================================
	// 함수 묶음: 타겟팅/조준/발사/인지/아이들/스폰-디스폰
	// =========================================================

	// 타겟팅/조준(메인) - C++ 한 곳에서 Yaw/Pitch 속도 제한
	void UpdateAimToTarget(float deltaSeconds);

	// LookAt(Yaw) 타깃과 Pitch 각도를 "속도 제한된 값"으로 산출
	void BuildSmoothedYawTargetAndPitch(const FVector& trueTargetWS, float deltaSeconds, FVector& outYawLookAtWS, float& outPitchDeg, float& outYawErrDeg);

	// 발사 게이트/LOS
	void UpdateFireGate(float deltaSeconds);
	bool HasLineOfFire(const FVector& from, const FVector& to) const;

	// 발사/이펙트
	void Fire();
	void SpawnBullet(const FVector& muzzleLocation, const FVector& direction);
	void PlayMuzzleFlash();
	void PlayTracer(const FVector& endPoint);

	// 잔탄 소진 처리
	void HandleOutOfAmmo();

	// 인지(Perception)
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
	void UpdateTargetSelection();

	// 아이들 스캔
	void EnsureIdleTimer();
	void OnIdleAimTimer();

	// 스폰/디스폰(간단 절차)
	void StartSpawnSimple();
	void StartDescentSimple();
	void UpdateSpawnDescentSimple(float deltaSeconds);

protected:
	// =========================================================
	// 변수 묶음: 컴포넌트
	// =========================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	class UCapsuleComponent* _capsule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	USkeletalMeshComponent* _mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	USceneComponent* _muzzlePoint;

	// =========================================================
	// 변수 묶음: 스펙/파라미터(사격/체력/사거리/탄 등)
	// =========================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _fireInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _range = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	int32 _maxAmmo = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	int32 _curAmmo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _maxHp = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	float _curHp = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	TSubclassOf<class AGunBulletBase> _bulletClass;

	// =========================================================
	// 변수 묶음: 타겟팅/조준(스펙)
	// =========================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _aimYawLimitDeg = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _aimPitchUpDeg = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _aimPitchDownDeg = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _yawSpeedDegPerSec = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _pitchSpeedDegPerSec = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _aimToleranceDeg = 2.0f;

	// 타겟팅에 사용할 본 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	FName _boneName_Yaw = TEXT("rotator");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	FName _boneName_Pitch = TEXT("gunhousing");

	// =========================================================
	// 변수 묶음: 타겟팅/조준(런타임 상태)
	// =========================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	float _aimYawDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	float _aimPitchDeg = 0.0f;

	// =========================================================
	// 변수 묶음: 인지(Perception)
	// =========================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	class UAIPerceptionComponent* _perception;

	UPROPERTY()
	class UAISenseConfig_Sight* _sightConfig;

	// =========================================================
	// 변수 묶음: 이펙트
	// =========================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	class UParticleSystemComponent* _muzzleFlashComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	class UNiagaraComponent* _tracerComponent;

	// =========================================================
	// 변수 묶음: 아이들 스캔
	// =========================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _idleScanInterval = 2.0f;

	// =========================================================
	// 변수 묶음: 스폰/디스폰(간단)
	// =========================================================
	float _spawnTargetZ = 0.0f;
	bool  _isRaised = false;

	// =========================================================
	// 변수 묶음: 런타임 캐시/타이머/기타
	// =========================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	AActor* _currentTarget = nullptr;

	FTimerHandle _fireTimerHandle;
	FTimerHandle _idleAimTimerHandle;

	bool  _lastWantsFire = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _losCheckInterval = 0.05f;

	float _losCooldown = 0.0f;
	bool  _cachedHasLOS = false;
	float _cosAimTol = 1.0f;
	float _cachedAimTolDeg = 0.0f;

	FVector _idleAimPointWS = FVector::ZeroVector;
};
