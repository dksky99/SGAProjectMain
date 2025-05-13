// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Throwable.h"
#include "TimedGrenadeBase.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API ATimedGrenadeBase : public AThrowable
{
	GENERATED_BODY()
	
public:
	virtual void StartCookingGrenade();
	virtual void UpdateCookingGrenade();
	virtual void Throw() override;
	virtual void ExplodeGrenade();

	bool IsFuseTimeRemaining();
	float GetRemainingFuseTime();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	float _totalFuseTime = 5.0f; // 터지기 까지 걸리는 시간

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	float _cookedTime = 0.0f; // 쿠킹 시간

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	float _explosionRadius = 300.0f; // 폭발 반경

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	float _explosionDamage = 100.0f; // 폭발 데미지

	bool _isExploded = false;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	UParticleSystem* _explosionEffect; // 폭발 이펙트

	FTimerHandle _explosionTimerHandle;
};
