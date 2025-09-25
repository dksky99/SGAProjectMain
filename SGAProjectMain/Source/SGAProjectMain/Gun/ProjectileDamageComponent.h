// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunDamageComponent.h"
#include "ProjectileDamageComponent.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UProjectileDamageComponent : public UGunDamageComponent
{
	GENERATED_BODY()
	
public:
	virtual void SetDamageData(const FGunData& gunData);

protected:
	virtual void DoFireShot(FVector fireLocation, FVector fireDirection);

	UPROPERTY()
	TSubclassOf<class AGunBulletBase> _projectileClass;
};
