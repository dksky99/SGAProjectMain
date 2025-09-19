// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GunDataTable.h"
#include "GunFireComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnFire);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SGAPROJECTMAIN_API UGunFireComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGunFireComponent();

	void SetFireModeData(TArray<EFireMode> fireModes);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool CanFire();
	void StartFire();
	void TryFire();
	void StopFire();

	FOnFire _fireEvent;

	void ChangeFireMode();
	EFireMode GetCurFireMode() { return _curFireMode; }

protected:
	void ResetBolt() { _canBoltFire = true; }

	UPROPERTY()
	AGunBase* _gun;

	TArray<EFireMode> _fireModes;
	EFireMode _curFireMode = EFireMode::FireAuto;
	int32 _curFireIndex = 0;
		
	FTimerHandle _fireTimer;
	FTimerHandle _boltActionTimer; // 볼트액션용

	int32 _burstCount = 3;
	bool _canBoltFire = true;

};
