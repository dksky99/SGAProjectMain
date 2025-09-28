// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileDamageComponent.h"
#include "../../GunBase.h"
#include "../../GunBulletBase.h"

void UProjectileDamageComponent::SetDamageData(const FGunData& gunData)
{
	Super::SetDamageData(gunData);

	_projectileClass = gunData._projectileClass;
}

void UProjectileDamageComponent::DoFireShot(FVector fireLocation, FVector fireDirection)
{
	FRotator fireRotation = fireDirection.Rotation();

	FActorSpawnParameters spawnParams;
	spawnParams.Instigator = GetOwner()->GetOwner<AHellDiver>(); // 헬다이버
	spawnParams.Owner = GetOwner(); // 총
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn; // 겹치면 움직여서, 아니면 억지로라도 생성

	AGunBulletBase* projectile = GetWorld()->SpawnActor<AGunBulletBase>(_projectileClass, fireLocation, fireRotation, spawnParams);
}
