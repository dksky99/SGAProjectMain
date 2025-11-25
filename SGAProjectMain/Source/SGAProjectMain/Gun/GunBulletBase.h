// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Data/GunDataTable.h"
#include "../Data/GunProjectileDataAsset.h"
#include "../Object/CDamageType.h"
#include "GunBulletBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBulletHit, EHitOutcome, hitOutcome);

UCLASS()
class SGAPROJECTMAIN_API AGunBulletBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGunBulletBase();
	

	//FBulletData GetBulletData() { return _bulletData; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnBulletOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnBulletHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void Explode();

	float CalculateSpeedFalloffMultiplier(float distance); // 총알 감속 계산
	EHitOutcome CalculateHitOutcome(int32 AV, const FHitResult& SweepResult, int32& AP);
	void ProcessHitOutcome(EHitOutcome outcome, const FHitResult& SweepResult);

	int32 SurfaceToAV(EPhysicalSurface surface);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void InitializeProjectile(FGunProjectileData data);

	UPROPERTY()
	FOnBulletHit _bulletHitEvent;

private:
	UPROPERTY(EditAnywhere, Category = "Game/GunData")
	FGunProjectileData _projectileData;

    UPROPERTY(VisibleAnywhere)
    class USphereComponent* _collisionComp;

    UPROPERTY(VisibleAnywhere)
    class UProjectileMovementComponent* _projectileMovement;
	
	// 중복 충돌 이벤트 방지용
	UPROPERTY()
	TSet<UPrimitiveComponent*> _hitComponents; 

	bool _isExploded = false;
	int32 _penetrationCount = 0;

	float _baseSpeed;
	float _moveDistance = 0.f;
	FVector _prevLoc;

	// 폭발 데미지 컴포넌트
	UPROPERTY(EditAnywhere, Category = "Game/GunBullet/Explosion")
	class UExplosionComponent* _explosionComponent;
};
