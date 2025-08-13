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

	void SpawnBullet(const FVector& muzzleLocation, const FVector& direction);

	// 외부에서 데미지를 받을 때 호출되는 오버라이드 함수
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void HandleOutOfAmmo();

	// 머즐 플래시 이펙트를 재생합니다.
	void PlayMuzzleFlash();

	// 트레이서 이펙트를 재생합니다.
	void PlayTracer(const FVector& EndPoint);


protected:
	// 터렛 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	USkeletalMeshComponent* _mesh;

	// 발사할 탄환 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	TSubclassOf<class AGunBulletBase> _bulletClass;

	// 탄환이 스폰될 머즐 포인트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	USceneComponent* _muzzlePoint;

	// 최대 사정거리 (cm 단위)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _range = 10000.0f;

	// 머즐 플래시용 ParticleSystemComponent을 미리 생성 → Activate/Deactivate로 재생
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	UParticleSystemComponent* _muzzleFlashComponent;

	// 트레이서용 NiagaraComponent를 미리 생성 → BeamEnd 파라미터 갱신 후 Activate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	class UNiagaraComponent* _tracerComponent;

	// 자동 사격 간격 (초 단위)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _fireInterval = 0.1f;

	// 최대 탄약 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	int32 _maxAmmo;

	// 현재 남은 탄약 수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	int32 _curAmmo;

	// 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Sentry")
	float _maxHp;

	// 현재 체력
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Sentry")
	float _curHp;

	// 자동 사격 타이머 핸들
	FTimerHandle _fireTimerHandle;
};
