// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGun.h"

#include "GunBulletBase.h"
#include "Kismet/GameplayStatics.h"
#include "../Character/HellDiver/HellDiver.h"

void AProjectileGun::Fire()
{
	if (!_projectileClass) return;
	Super::Fire();
}

void AProjectileGun::ExecuteShot()
{
	FVector muzzleLocation = _gunMesh->GetSocketLocation(TEXT("Muzzle"));
	FVector fireDirection = _gunMesh->GetSocketRotation(TEXT("Muzzle")).Vector();

	// 조준하고 있지 않을 경우 탄퍼짐
	if (!_owner->GetStateComponent()->IsAiming())
	{
		fireDirection = FMath::VRandCone(fireDirection, FMath::DegreesToRadians(_gunData._shakeAmount));
	}
	FRotator fireRotation = fireDirection.Rotation();

	FActorSpawnParameters spawnParams;
	spawnParams.Instigator = GetOwner<APawn>(); // 헬다이버
	spawnParams.Owner = this; // 총
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn; // 겹치면 움직여서, 아니면 억지로라도 생성

	AGunBulletBase* projectile = GetWorld()->SpawnActor<AGunBulletBase>(_projectileClass, muzzleLocation, fireRotation, spawnParams);
}