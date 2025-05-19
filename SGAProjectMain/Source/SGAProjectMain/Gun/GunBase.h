// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GunBase.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FAmmoChanged, int, int);

UENUM(BlueprintType)
enum class EFireMode : uint8
{
	Auto,
	Semi,        
	Burst,
};

USTRUCT(BlueprintType)
struct FGunData // : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _baseDamage = 80.0f;

	// �߻� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _fireInterval = 60.0f / 640.0f;

	// ź��
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _maxAmmo = 45;

	// �ݵ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _recoil = 14.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _verticalRecoil = 5.f;        // ���� �ݵ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _horizontalRecoil = 6.f;      // ���� �ݵ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _shakeAmount = 4.f;           // ��鸲
	
	// �Ÿ��� ���� ������ ���ҷ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _falloff25 = 0.04f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _falloff50 = 0.072f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _falloff100 = 0.133f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<EFireMode> _fireModes = { EFireMode::Auto, EFireMode::Burst, EFireMode::Semi};
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

	void InitializeGun();
	void ActivateGun();
	void DeactivateGun();
	void AttachToHand();
	void Reload();

	float CalculateDamage(float distance); // �Ÿ��� ���� ������ ����

	void TickRecoil(float DeltaTime); // �����ӿ� ���� �ݵ�
	void ApplyFireRecoil(); // ��ݿ� ���� �ݵ�
	float GetRecoilMultiplier(); // ���¿� ���� �ݵ� ����

	FVector CalculateHitPoint();
	
	void EnterGunSettingMode();
	void ExitGunSettingMode();
	void ChangeFireMode();

	FAmmoChanged _ammoChanged;

private:
	UPROPERTY(EditAnywhere, Category = "Game/Gun")
	TObjectPtr<USkeletalMeshComponent> _mesh;

	UPROPERTY(VisibleAnywhere, Category = "Game/Gun")
	class AHellDiver* _owner;

	UPROPERTY(EditAnywhere, Category = "Game/GunData")
	FGunData _gunData;

	FTimerHandle _fireTimer;

	int32 _curAmmo;

	FVector _hitPoint;

	FRotator _recoilOffset = FRotator::ZeroRotator;
	float _totalVerticalRecoil = 0.f;
	float _maxVerticalRecoil;
	float _maxHorizontalRecoil;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Gun", meta = (AllowPrivateAccess = "true"))
	EFireMode _fireMode = EFireMode::Auto;
	int _fireIndex = 0;
	int _burstCount = 3;

	UPROPERTY()
	class AImpactMarker* _marker;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AImpactMarker> _impactMarkerClass;

	UPROPERTY()
	UUserWidget* _crosshair;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> _crosshairClass;

	bool _isAiming = false;
};
