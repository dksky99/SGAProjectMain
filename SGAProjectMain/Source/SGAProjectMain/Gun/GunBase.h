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

	// �߻� ����
	float _fireInterval = 60.0f / 640.0f;

	// ź��
	int32 _maxAmmo = 45;

	// �ݵ�
	float _recoil = 14.f;
	float _verticalRecoil = 5.f;        // ���� �ݵ�
	float _horizontalRecoil = 6.f;      // ���� �ݵ�
	float _shakeAmount = 4.f;           // ��鸲
	
	// �Ÿ��� ���� ������ ���ҷ�
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

	float CalculateDamage(float distance); // �Ÿ��� ���� ������ ����

	void TickRecoil(float DeltaTime); // �����ӿ� ���� �ݵ�
	void ApplyFireRecoil(); // ��ݿ� ���� �ݵ�
	float GetRecoilMultiplier(); // ���¿� ���� �ݵ� ����

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
