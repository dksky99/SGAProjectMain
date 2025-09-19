// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GunDataTable.h"
#include "../Object/Item/ItemBase.h"
#include "GunBase.generated.h"

UCLASS()
class SGAPROJECTMAIN_API AGunBase : public AItemBase
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

	virtual bool CanFire();

	virtual void StartFire();
	virtual void StopFire();

protected:
	virtual void Fire();
	virtual void ExecuteShot(); // 히트스캔

public:
	virtual void StartAiming();
	virtual void StopAiming();

	void InitializeGun();
	void ActivateGun();
	void DeactivateGun();
	void AttachToHand();

	virtual void Reload();
	virtual void OnReloadSectionEnded(); // 장전 몽타주 끝날 때마다 호출
	void CancelReload();

	void RefillMag();

	float CalculateDamage(float distance); // 거리에 따른 데미지 감소

	//void TickRecoil(float DeltaTime);
	void RecoverRecoil(float DeltaTime); // 반동 복구
	void ApplyFireRecoil(); // 사격에 따른 반동
	float GetRecoilMultiplier(); // 상태에 따른 반동 정도

	FHitResult GetHitResult(ECollisionChannel TraceChannel);

	void ChangeFireMode();
	void ChangeTacticalLightMode();
	void ChangeScopeMode();

	void UseLaserPoint(FVector hitPoint);
	void UseTacticalLight(bool isAiming);

	virtual void PickupItem(AHellDiver* player) override;

	virtual void PlayFireEffect();

	const FGunData& GetGunData() { return _gunData; }
	void SetGunData(const FGunData& gunData);

	class UGunFireComponent* GetFireComponent() { return _fireComp; }
	class UGunAmmoComponent* GetAmmoComponent() { return _ammoComp; }

	EFireMode GetCurFireMode();
	int32 GetCurAmmo();
	ETacticalLightMode GetCurLightMode() { return _tacticalLightMode; }
	int32 GetCurScopeMode() { return _scopeMode; }
	USkeletalMeshComponent* GetMesh() { return _gunMesh; }
	class AHellDiver* GetOwnerCharacter() { return _owner; }
		
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FTransform GetMuzzleTrans();
	FVector GetMuzzleLoc();
	FRotator GetMuzzleRot();

	FTransform GetLeftHandleTrans();

protected:
	UPROPERTY(EditAnywhere, Category = "Game/Gun")
	TObjectPtr<USkeletalMeshComponent> _gunMesh;

	UPROPERTY(VisibleAnywhere, Category = "Game/GunComponent")
	class UGunFireComponent* _fireComp;		// 발사 관련 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Game/GunComponent")
	class UGunAmmoComponent* _ammoComp;		// 탄약 관련 컴포넌트


	UPROPERTY(VisibleAnywhere, Category = "Game/Gun")
	class AHellDiver* _owner;

	UPROPERTY(VisibleInstanceOnly, Category = "Game/GunData")
	FGunData _gunData;

	UPROPERTY(EditAnywhere, Category = "Game/Gun")
	int32 _gunID; // gunData 초기화용

	bool _isActive = false;

	FRotator _recoilToRecover = FRotator::ZeroRotator;
	// 반동 정도 조절을 위한 수치 -> 테스트 필요
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Recoil", meta = (AllowPrivateAccess = "true"))
	float _recoilRecoverSpeed = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Recoil", meta = (AllowPrivateAccess = "true"))
	float _verticalRecoilDamp = 5.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Recoil", meta = (AllowPrivateAccess = "true"))
	float _horizontalRecoilDamp = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* _fireMontage;
	UPROPERTY()
	class AImpactMarker* _marker;

	UPROPERTY()
	UUserWidget* _crosshair;

	UPROPERTY(VisibleAnywhere, Category = "Game/Gun")
	class USpotLightComponent* _tacticalLight;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Gun", meta = (AllowPrivateAccess = "true"))
	ETacticalLightMode _tacticalLightMode = ETacticalLightMode::LightOff;
	int32 _lightIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Gun", meta = (AllowPrivateAccess = "true"))
	int32 _scopeMode;
	int32 _scopeIndex;

	UPROPERTY()
	class UNiagaraComponent* _laserpointer;
	UPROPERTY()
	class UNiagaraComponent* _laserImpact;

	UPROPERTY(EditAnywhere, Category = "Game/Gun")
	class UNiagaraSystem* _fireNS;
	UPROPERTY()
	class UNiagaraComponent* _fireEffect;
	UPROPERTY(EditAnywhere, Category = "Game/Gun")
	class UNiagaraSystem* _shellEjectNS;
	UPROPERTY()
	class UNiagaraComponent* _shellEjectEffect;
};
