// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGun.h"

#include "GunBulletBase.h"
#include "Kismet/GameplayStatics.h"
#include "../Character/HellDiver/HellDiver.h"

void AProjectileGun::Fire()
{
	// 탄창, 약실 모두 비었음
	if (_curAmmo <= 0 && !_isChamberLoaded)
	{
		UE_LOG(LogTemp, Log, TEXT("Mag Empty"));
		StopFire();
		return;
	}

	auto camera = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
	if (!camera) return;

	if (_fireMode == EFireMode::FireBoltAction)
	{
		if (!_canFire) return;

		_canFire = false;
		GetWorldTimerManager().SetTimer(_boltActionTimer, this, &AGunBase::ResetCanFire, _gunData._fireInterval, false);
	}

	if (_fireMode == EFireMode::FireBurst)
	{
		if (_burstCount <= 0)
		{
			StopFire();
			return;
		}
		_burstCount--;
	}

	ApplyFireRecoil();

	// 여기서부터 GunBase와 차이 발생
	FVector muzzleLocation = _mesh->GetSocketLocation(TEXT("Muzzle"));
	FVector fireDirection = _mesh->GetSocketRotation(TEXT("Muzzle")).Vector();

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

	/*if (projectile)
	{
		projectile->InitializeProjectile();
	}*/
	// 여기까지 GunBase와 차이

	if (_curAmmo > 0) // 탄창에 탄약이 남아있을 경우
	{
		_curAmmo--;
	}
	else // 약실에만 남아있을 경우
	{
		_isChamberLoaded = false;
	}

	if (_ammoChanged.IsBound())
		_ammoChanged.Broadcast(_curAmmo, _gunData._maxAmmo);
}
