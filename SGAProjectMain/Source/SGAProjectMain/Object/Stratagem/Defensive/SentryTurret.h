// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GenericTeamAgentInterface.h"
#include "SentryTurret.generated.h"

UCLASS()
class SGAPROJECTMAIN_API ASentryTurret : public AActor, public IGenericTeamAgentInterface
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

	// 각도 계산
	float CalcYaw_Sentry();
	float CalcPitch_Sentry();

	// 회전 속도 제한 적용(보간) 함수
	void ApplyAimSpeedLimit(float deltaSeconds, float targetYawDeg, float targetPitchDeg);

	// 최단 각도 차 기반 총구 정렬 
	bool IsAngleAligned(float currentDeg, float targetDeg, float toleranceDeg) const;
	
	// 발사 게이트/LOS
	void UpdateFireGate(float deltaSeconds);

	// 발사/이펙트
	void Fire();
	void SpawnBullet(const FVector& muzzleLocation, const FVector& direction);
	void InitNiagaraEffects();
	void PlayMuzzleFX();
	void PlayCasingFX();

	// 잔탄 소진 처리
	void HandleOutOfAmmo();

	// 인지(Perception)
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
	void UpdateTargetSelection();

	// 아이들 스캔
	void EnsureIdleTimer();
	void OnIdleAimTimer();

	// 팀 인터페이스 오버라이드
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId) override { _teamId = NewTeamId; }
	virtual FGenericTeamId GetGenericTeamId() const override { return _teamId; }

	// 적 판정(상대가 팀 인터페이스를 구현하면 TeamId 비교, 아니면 Neutral 취급)
	bool IsEnemyActor(const AActor* Other) const;

	// 스폰/디스폰
	void StartSpawn();
	void StartDescent();
	void UpdateSpawnDescent(float deltaSeconds);

protected:
	// =========================================================
	// 변수 묶음: 컴포넌트
	// =========================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	USkeletalMeshComponent* _mesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	USceneComponent* _muzzlePoint = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	USentryAnimInstance* _anim = nullptr;

	// =========================================================
	// 변수 묶음: 스펙/파라미터(사격/체력/사거리/탄 등)
	// =========================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _fireInterval = 0.1f;

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

	FTimerHandle _fireTimerHandle;

	bool  _lastWantsFire = false;

	// =========================================================
	// 변수 묶음: 타겟팅/조준(스펙)
	// =========================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	AActor* _currentTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _yawSpeedDegPerSec = 360.0f;		// Yaw 회전속도(도/초)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _PitchSpeedDegPerSec = 180.0f;	// Pitch 회전속도(도/초)

	// Pitch 한계(상/하)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _aimPitchUpDeg = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _aimPitchDownDeg = 20.0f;

	// 현재 본 각도(프레임 간 상태 유지)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	float _rotatorZ_CurrentDeg = 0.0f;     // 수평 회전부(Rotator)의 Z

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	float _gunHousingZ_CurrentDeg = 0.0f;  // 포신 하우징(GunHousing)의 Z

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _aimToleranceDeg = 2.0f; // 발사 게이트에 사용

	// 타겟팅에 사용할 본 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	FName _boneName_Yaw = TEXT("rotator");      // 수평(Yaw) 부모

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	FName _boneName_Pitch = TEXT("gunhousing"); // 수직(Pitch) 자식

	float _cosAimTol = 1.0f;
	float _cachedAimTolDeg = 0.0f;

	// =========================================================
	// 변수 묶음: 인지(Perception)
	// =========================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	class UAIPerceptionComponent* _perception = nullptr;

	UPROPERTY()
	class UAISenseConfig_Sight* _sightConfig = nullptr;

	// =========================================================
	// 변수 묶음: 아이들 스캔
	// =========================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _idleScanInterval = 2.0f;

	FVector _idleAimPointWS = FVector::ZeroVector;

	
	FTimerHandle _idleAimTimerHandle;

	// =========================================================
	// 변수 묶음: 이펙트
	// =========================================================
	
	// 이펙트 에셋
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	class UNiagaraSystem* _muzzleNS = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	class UNiagaraSystem* _casingNS = nullptr;

	// 소켓 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	FName _casingSocketName = "ejectSocket";  // ejector 본에 단 소켓 권장

	// 머즐 풀
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	int32 _muzzlePoolSize = 8;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	TArray<class UNiagaraComponent*> _muzzlePool;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	int32 _muzzlePoolIndex = 0;

	// 탄피 풀
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	int32 _casingPoolSize = 12;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	TArray<class UNiagaraComponent*> _casingPool;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	int32 _casingPoolIndex = 0;


	// =========================================================
	// 변수 묶음: 스폰/디스폰
	// =========================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	float _spawnTargetZ = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	bool _isRaised = false;     // 상승 완료(전투 가능)

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	bool _isSinking = false;    // 하강(퇴장) 중

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	bool _isTransitionalAlign = false;  // 총구 정렬 단계

	// =========================================================
	// 변수 묶음: 팀/소속
	// =========================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	uint8 _teamId = 1;

};
