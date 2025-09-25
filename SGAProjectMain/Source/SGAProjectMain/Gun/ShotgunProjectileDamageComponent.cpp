// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotgunProjectileDamageComponent.h"
#include "../Gun/GunBase.h"
#include "../Gun/GunBulletBase.h"

void UShotgunProjectileDamageComponent::SetDamageData(const FGunData& gunData)
{
	Super::SetDamageData(gunData);

	_shotgunData = gunData._shotgunData;
}

void UShotgunProjectileDamageComponent::DoFireShot(FVector fireLocation, FVector fireDirection)
{
	for (int i = 0; i < _shotgunData._pelletCount; i++)
	{
		FVector pelletDirection = FMath::VRandCone(fireDirection, _shotgunData._horizontalSpread * 0.001f, _shotgunData._verticalSpread * 0.001f);
		
		Super::DoFireShot(fireLocation, pelletDirection);
	}
}

