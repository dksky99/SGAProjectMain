// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileDamageComponent.h"
#include "ShotgunProjectileDamageComponent.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UShotgunProjectileDamageComponent : public UProjectileDamageComponent
{
	GENERATED_BODY()

public:
	virtual void SetDamageData(const FGunData& gunData) override;

protected:
	virtual void DoFireShot(FVector fireLocation, FVector fireDirection) override;

	// 샷건 관련 데이터
	FShotgunData _shotgunData;
};
