// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SentryTurret.generated.h"

USTRUCT(BlueprintType)
struct FSentryData
{
	GENERATED_BODY()

	FSentryData()
		: _baseDamage(20.0f)
		, _falloff25(0.1f)
		, _falloff50(0.2f)
		, _falloff100(0.5f)
		, _fireInterval(0.1f)
	{}

	// 한 발당 기본 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _baseDamage;

	// 25m 거리까지의 최대 데미지 감쇠 비율 (0~1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _falloff25;

	// 50m 거리까지의 최대 데미지 감쇠 비율 (0~1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _falloff50;

	// 100m 이상 거리에서의 최대 데미지 감쇠 비율 (0~1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _falloff100;

	// 발사 간격(초 단위)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _fireInterval;
};

UCLASS()
class SGAPROJECTMAIN_API ASentryTurret : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASentryTurret();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 센트리가 한 방향으로 자동 사격을 시작합니다.
	UFUNCTION(BlueprintCallable, Category = "Game/Stratagem/Sentry")
	void AIStartFire();

	// 센트리가 자동 사격을 중단합니다.
	UFUNCTION(BlueprintCallable, Category = "Game/Stratagem/Sentry")
	void AIStopFire();

	// 실제로 한 발을 발사합니다. (AIStartFire에 의해 Timer로 호출)
	void Fire();

	// 라인트레이스 판정 및 거리 계산 후 HitResult를 반환합니다.
	FHitResult GetHitResult() const;

	// 거리(dist, 단위: m)에 따라서 데미지를 계산하여 반환합니다.
	float CalculateDamage(float distance) const;

	// 맞은 액터에게 ApplyDamage를 호출 함수 
	void ApplyHitDamage(const FHitResult& hit, float damage);

	// 외부에서 데미지를 받을 때 호출되는 오버라이드 함수
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void HandleOutOfAmmo();

	// 머즐 플래시 이펙트를 재생합니다.
	void PlayMuzzleFlash();

	// 트레이서 이펙트를 재생합니다.
	void PlayTracer(const FVector& EndPoint);


protected:
	// 터렛 몸통(박스 등)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	UStaticMeshComponent* _bodyMesh;

	// 터렛 총구(원통)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	UStaticMeshComponent* _barrelMesh;

	// 최대 사정거리 (cm 단위)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _range = 10000.0f;

	// 머즐 플래시용 ParticleSystemComponent을 미리 생성 → Activate/Deactivate로 재생
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	UParticleSystemComponent* _muzzleFlashComponent;

	// 트레이서용 NiagaraComponent를 미리 생성 → BeamEnd 파라미터 갱신 후 Activate
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	UNiagaraComponent* _tracerComponent;

	// 최대 탄약 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	int32 _maxAmmo;

	// 현재 남은 탄약 수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	int32 _curAmmo;

	// 센트리 사격 데이터 (데미지, 거리별 감쇠, 발사 간격)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	FSentryData _sentryData;

	// 자동 사격 타이머 핸들
	FTimerHandle _fireTimerHandle;

	// 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _maxHp;

	// 현재 체력
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	float _curHp;
};
