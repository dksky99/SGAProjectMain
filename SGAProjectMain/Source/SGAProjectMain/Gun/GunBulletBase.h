// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Data/GunDataTable.h"
#include "../Data/GunProjectileDataAsset.h"
#include "GunBulletBase.generated.h"

UENUM(BlueprintType)
enum class EBulletType : uint8
{
	Standard,
	Explosive
};

UENUM(BlueprintType)
enum class EHitOutcome : uint8
{ 
	OverPenetrating, 
	FullPenetrate, 
	Penetrate, 
	Ricochet 
};

USTRUCT(BlueprintType)
struct FBulletData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBulletType _type = EBulletType::Standard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _initialSpeed = 3500.f; // 초기 속도

	// 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _baseDamage = 3500.f; // 기본 데미지

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _vsDurableDamage = 3500.f; // 내구 데미지

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//float _explosionDamage = 150.f; // 폭발 데미지

	// 관통력
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPenetrateTrait _basePenetrateTrait = EPenetrateTrait::AntiTank;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPenetrateTrait _explosionPenetrateTrait = EPenetrateTrait::AntiTank;

	// 거리에 따른 데미지 감소량
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _falloff25 = 0.04f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _falloff50 = 0.072f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _falloff100 = 0.133f;

	// 폭발 범위
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//float _innerRadius = 150.f; // 중심 범위
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//float _outerRadius = 600.f; // 전체 범위

};

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
	EHitOutcome CalculateHitOutcome(int32 AV, const FHitResult& SweepResult);
	void ProcessHitOutcome(EHitOutcome outcome, const FHitResult& SweepResult);

	int32 SurfaceToAV(EPhysicalSurface surface)
	{
		switch (surface)
		{
		case SurfaceType1: return 0; // AV0_UnarmoredI
		case SurfaceType2: return 1; // AV1_UnarmoredII
		case SurfaceType3: return 2; // AV2_Light
		case SurfaceType4: return 3; // AV3_Medium
		case SurfaceType5: return 4; // AV4_Heavy
		case SurfaceType6: return 5; // AV5_TankI
		case SurfaceType7: return 6; // AV6_TankII
		default:           return 0;
		}
	}

public:
	void InitializeProjectile(FGunProjectileData data);

private:
	//UPROPERTY(EditAnywhere, Category = "Game/GunData")
	//FBulletData _bulletData;

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
