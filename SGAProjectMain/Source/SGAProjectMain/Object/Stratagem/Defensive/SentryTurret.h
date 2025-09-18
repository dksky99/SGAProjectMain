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

	// 외부에서 강제로 타깃을 지정하고 싶을 때 사용
	void SetTargetActor(AActor* target);

	// 자동 사격 제어
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
	// 조준 각도 계산 → AnimBP 전달
	void UpdateAimToTarget(float deltaSeconds);

	// Perception 콜백
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	// 타깃 유지/재선정
	void UpdateTargetSelection();

	// 조준 완료 + LOS 확인 → 발사/중지
	void UpdateFireGate(float deltaSeconds);

	// LOS 체크
	bool HasLineOfFire(const FVector& from, const FVector& to) const;

	// 발사 루틴
	void Fire();
	void SpawnBullet(const FVector& muzzleLocation, const FVector& direction);
	void PlayMuzzleFlash();
	void PlayTracer(const FVector& endPoint);
	void HandleOutOfAmmo();

	// 아이들 타이머 on/off 및 콜백
	void EnsureIdleTimer();
	void OnIdleAimTimer();

	// 몽터주 제어
	UFUNCTION()
	void OnDeployMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void PlayDeployMontage(bool bForward);
	bool IsDeployMontagePlaying() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	class UCapsuleComponent* _capsule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	USkeletalMeshComponent* _mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	USceneComponent* _muzzlePoint;

	// 머즐 플래시용 ParticleSystemComponent을 미리 생성 -> Activate/Deactivate로 재생
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	class UParticleSystemComponent* _muzzleFlashComponent;

	// 트레이서용 NiagaraComponent를 미리 생성 -> BeamEnd 파라미터 갱신 후 Activate
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	class UNiagaraComponent* _tracerComponent;

	// Perception
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	class UAIPerceptionComponent* _perception;

	UPROPERTY()
	class UAISenseConfig_Sight* _sightConfig;

protected:
	// 발사할 탄환 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	TSubclassOf<class AGunBulletBase> _bulletClass;

	// 애님 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	UAnimMontage* _deployMontage = nullptr;

	// 자동 사격 간격 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _fireInterval = 0.1f;

	// 최대 사정거리 (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _range = 10000.0f;

	// 최대/현재 탄약
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	int32 _maxAmmo = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	int32 _curAmmo = 0;

	// 최대/현재 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _maxHp = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	float _curHp = 100.0f;

	// 자동 사격 타이머 핸들
	FTimerHandle _fireTimerHandle;

	// 아이들 상태일 시 회전 타이머 핸들
	FTimerHandle _idleAimTimerHandle;

	// 현재 타깃
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	AActor* _currentTarget = nullptr;

	// 아이들 상태일 시 회전 주기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _idleScanInterval = 2.0f;

protected:
	// 조준 한계/보간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _aimYawLimitDeg = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _aimPitchUpDeg = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _aimPitchDownDeg = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _aimInterpSpeed = 12.0f;

	// 현재 조준 각도(디버그/AnimBP 확인용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	float _aimYawDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	float _aimPitchDeg = 0.0f;

	// 발사 게이트 허용 오차(도)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _aimToleranceDeg = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _yawSpeedDegPerSec = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _pitchSpeedDegPerSec = 120.0f;

	// LOS 체크 레이트 리밋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _losCheckInterval = 0.05f;

protected:
	// 발사 토글 캐시: 상태 변경시에만 Start/Stop 호출
	bool _lastWantsFire = false;

	float _losCooldown = 0.0f;
	bool  _cachedHasLOS = false;

	// 허용오차 코사인 캐시
	float _cosAimTol = 1.0f;
	float _cachedAimTolDeg = 0.0f;

	// 아이들 상태일 시 회전 목표
	FVector _idleAimPointWS = FVector::ZeroVector;

	// 몽타주가 재생된적 있나
	bool _hasPlayedDeployMontage = false;

};
