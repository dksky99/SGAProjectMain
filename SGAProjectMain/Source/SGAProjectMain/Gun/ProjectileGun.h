// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunBase.h"
#include "ProjectileGun.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AProjectileGun : public AGunBase
{
	GENERATED_BODY()

protected:
	virtual void Fire() override;
	virtual void ExecuteShot() override;
	
	UPROPERTY(EditAnywhere, Category = "Game/Gun")
	TSubclassOf<class AGunBulletBase> _projectileClass;
};
