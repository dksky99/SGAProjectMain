// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GunBase.generated.h"

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

private:
	// TODO (총별 속성 - 탄약, 반동...)

	FTimerHandle _fireTimer; // 발사 타이머 관리
	float _fireInterval = 0.2f; // 발사 간격


	bool _isHit = false;
	FVector start;
	FVector _hitPoint;

	UPROPERTY()
	class AImpactMarker* _marker;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AImpactMarker> _impactMarkerClass;

	bool _isAiming = false;
};
