// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileDamageComponent.h"
#include "../../GunBase.h"

void UProjectileDamageComponent::SetDamageData(const FGunData& gunData)
{
	Super::SetDamageData(gunData);

	if (!gunData._projectileDataAsset) return;

	_projectileClass = gunData._projectileDataAsset->_projectileClass;
	_projectileData = gunData._projectileDataAsset->_projectileData;
}

void UProjectileDamageComponent::DoFireShot(FVector fireLocation, FVector fireDirection)
{
	FRotator fireRotation = fireDirection.Rotation();

	FActorSpawnParameters spawnParams;
	spawnParams.Instigator = GetOwner()->GetOwner<AHellDiver>(); // 헬다이버
	spawnParams.Owner = GetOwner(); // 총
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn; // 겹치면 움직여서, 아니면 억지로라도 생성

	if (!_projectileClass) return;

	AGunBulletBase* projectile = GetWorld()->SpawnActor<AGunBulletBase>(_projectileClass, fireLocation, fireRotation, spawnParams);
	// 데미지 계산은 총알 쪽에서 이루어진다
	projectile->InitializeProjectile(_projectileData);

	if (auto gun = Cast<AGunBase>(GetOwner()))
		projectile->_bulletHitEvent.AddDynamic(gun, &AGunBase::ShowHitMarker);
}
