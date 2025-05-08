// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GunBase.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FAmmoChanged, int, int);

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

	void Reload();

	FAmmoChanged _ammoChanged;

private:
	// TODO (ÃÑº° ¼Ó¼º - Åº¾à, ¹Ýµ¿...)

	// ¹ß»ç °£°Ý
	FTimerHandle _fireTimer;
	float _fireInterval = 0.2f;

	// Åº¾à
	int32 _curAmmo;
	int32 _maxAmmo = 45; 

	bool _isHit = false;
	FVector start;
	FVector _hitPoint;

	UPROPERTY()
	class AImpactMarker* _marker;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AImpactMarker> _impactMarkerClass;

	bool _isAiming = false;
};
