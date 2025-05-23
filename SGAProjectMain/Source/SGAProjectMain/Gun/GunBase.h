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
	BoltAction
};

UENUM(BlueprintType)
enum class EReloadStage : uint8
{
	None,
	RemoveMag,
	InsertMag,
	CloseBolt,
	RoundsReload // 한 발씩 장전
};

UENUM(BlueprintType)
enum class ETacticalLightMode : uint8
{
	Auto,
	On,
	Off
};

USTRUCT(BlueprintType)
struct FGunData // : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _baseDamage = 80.0f;

	// 발사 간격
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _fireInterval = 60.0f / 640.0f;

	// 탄약
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _maxAmmo = 45;

	// 인체공학성
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _ergo = 54;

	// 반동
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _recoil = 14.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _verticalRecoil = 5.f;        // 수직 반동
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _horizontalRecoil = 6.f;      // 수평 반동
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _shakeAmount = 4.f;           // 흔들림
	
	// 거리에 따른 데미지 감소량
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _falloff25 = 0.04f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _falloff50 = 0.072f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _falloff100 = 0.133f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
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
	void ChangeReloadStage(); // 장전 몽타주 끝날 때마다 호출
	void CancelReload();

	float CalculateDamage(float distance); // 거리에 따른 데미지 감소

	void TickRecoil(float DeltaTime); // 움직임에 따른 반동
	void ApplyFireRecoil(); // 사격에 따른 반동
	float GetRecoilMultiplier(); // 상태에 따른 반동 정도

	FVector CalculateHitPoint();
	
	void EnterGunSettingMode();
	void ExitGunSettingMode();
	void ChangeFireMode();
	void ChangeTacticalLightMode();

	void UseLaserPoint(FVector hitPoint);
	void UseTacticalLight(bool isAiming);

	void ResetCanFire() { _canFire = true; }

	FAmmoChanged _ammoChanged;

private:
	UPROPERTY(EditAnywhere, Category = "Game/Gun")
	TObjectPtr<USkeletalMeshComponent> _mesh;

	UPROPERTY(VisibleAnywhere, Category = "Game/Gun")
	class AHellDiver* _owner;

	UPROPERTY(EditAnywhere, Category = "Game/GunData")
	FGunData _gunData;

	bool _isActive = false;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Gun", meta = (AllowPrivateAccess = "true"))
	EReloadStage _reloadStage = EReloadStage::None;
	UPROPERTY(VisibleAnywhere, Category = "Game/Gun")
	bool _isChamberLoaded = false; // 약실에 탄이 남았는지

	UPROPERTY()
	class AImpactMarker* _marker;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AImpactMarker> _impactMarkerClass;

	UPROPERTY()
	UUserWidget* _crosshair;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> _crosshairClass;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Gun", meta = (AllowPrivateAccess = "true"))
	class UNiagaraSystem* _laserFX;
	UPROPERTY()
	class UNiagaraComponent* _laserpointer;


	UPROPERTY(VisibleAnywhere, Category = "Game/Gun")
	class USpotLightComponent* _tacticalLight;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Gun", meta = (AllowPrivateAccess = "true"))
	ETacticalLightMode _tacticalLightMode = ETacticalLightMode::Auto;

	// 볼트액션용
	FTimerHandle _boltActionTimer;
	bool _canFire = true;
};
