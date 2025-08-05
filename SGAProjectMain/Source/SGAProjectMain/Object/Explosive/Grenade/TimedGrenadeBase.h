// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../Throwable.h"
#include "TimedGrenadeBase.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API ATimedGrenadeBase : public AThrowable
{
	GENERATED_BODY()
	
public:
	ATimedGrenadeBase();

	virtual void StartCookingGrenade();
	virtual void UpdateCookingGrenade();
	virtual void Throw(FVector direction) override;
	virtual void ExplodeGrenade();

	bool IsFuseTimeRemaining();
	float GetRemainingFuseTime();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Grenade")
	float _totalFuseTime = 5.0f; // 터지기 까지 걸리는 시간

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Grenade")
	float _cookedTime = 0.0f; // 쿠킹 시간

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Grenade/Explosion")
	class UExplosionComponent* _explosionComponent;

	bool _isExploded = false;

	FTimerHandle _explosionTimerHandle;
};
