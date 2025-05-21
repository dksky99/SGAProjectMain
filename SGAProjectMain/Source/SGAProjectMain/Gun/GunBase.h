// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GunBase.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FAmmoChanged, int, int);

USTRUCT(BlueprintType)
struct FGunData // : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _baseDamage = 80.0f;

	// 발사 간격
	float _fireInterval = 60.0f / 640.0f;

	// 탄약
	int32 _maxAmmo = 45;

	// 반동
	float _recoil = 14.f;
	float _verticalRecoil = 5.f;        // 수직 반동
	float _horizontalRecoil = 6.f;      // 수평 반동
	float _shakeAmount = 4.f;           // 흔들림
	
	// 거리에 따른 데미지 감소량
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _falloff25 = 0.04f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _falloff50 = 0.072f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _falloff100 = 0.133f;
};

UCLASS()
class SGAPROJECTMAIN_API AGunBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGunBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void StartFire();
	virtual void Fire();
	virtual void StopFire();

	virtual void StartAiming();
	virtual void StopAiming();

	void UpdateGun();
	void Reload();

	float CalculateDamage(float distance); // 거리에 따른 데미지 감소

	void TickRecoil(float DeltaTime); // 움직임에 따른 반동
	void ApplyFireRecoil(); // 사격에 따른 반동
	float GetRecoilMultiplier(); // 상태에 따른 반동 정도

	FVector CalculateHitPoint();

	FAmmoChanged _ammoChanged;

private:
	UPROPERTY(EditAnywhere, Category = "ItemData")
	FGunData _gunData;

	FTimerHandle _fireTimer;

	int32 _curAmmo;

	FVector _hitPoint;

	FRotator _recoilOffset = FRotator::ZeroRotator;
	float _totalVerticalRecoil = 0.f;
	float _maxVerticalRecoil;
	float _maxHorizontalRecoil;

	UPROPERTY()
	class AImpactMarker* _marker;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AImpactMarker> _impactMarkerClass;

	UPROPERTY()
	UUserWidget* _crosshair;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> _crosshairClass;

	bool _isAiming = false;
};
