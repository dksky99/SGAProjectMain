// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../GunDataTable.h"
#include "GunAmmoComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAmmoChanged, int, int);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSpareChanged, int, int);

UCLASS( BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SGAPROJECTMAIN_API UGunAmmoComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGunAmmoComponent();

	void SetAmmoData(const FGunData& gunData);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual bool CanFire();
	virtual void ConsumeAmmo();

	virtual bool CanReload();
	virtual void Reload();
	virtual void OnReloadSectionEnded(); // 장전 몽타주 끝날 때마다 호출
	void OnReloadMontageEnded(class UAnimMontage* Montage, bool bInterrupted);
	void CancelReload();
	void FinishReload();

protected:
	int32 PickReloadSection();
	void ChangeReloadStage();

public:
	void RefillSpare();

	void BroadcastAmmoAndSpareChanged();
	FOnAmmoChanged _ammoChanged;
	FOnSpareChanged _spareChanged;

	int32 GetCurAmmo() { return _isChamberLoaded ? _curAmmo + 1 : _curAmmo; };

protected:
	UPROPERTY()
	class AGunBase* _gun;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Gun")
	EReloadType _reloadType = EReloadType::Magazine;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Gun")
	bool _needAmmoBag = false; // 재장전할 때 가방이 필요한지

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Gun")
	EReloadStage _reloadStage = EReloadStage::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Animation")
	UAnimMontage* _reloadMontage;

	int32 _maxAmmo;
	int32 _curAmmo;

	int32 _maxSpare;
	int32 _curSpare;
	int32 _refillSpareAmount; // 탄창 보충량

	UPROPERTY(VisibleAnywhere, Category = "Game/Gun")
	bool _isChamberLoaded = false; // 약실에 탄이 남았는지

	
};
