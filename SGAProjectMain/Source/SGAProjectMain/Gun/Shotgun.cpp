// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"

#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "../Character/HellDiver/HellDiver.h"

void AShotgun::Fire()
{
	FColor drawColor = FColor::Green;

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

	// 총구 위치에서 총구가 향하는 방향으로 발사
	FVector muzzleLocation = _mesh->GetSocketLocation(TEXT("Muzzle"));
	FVector fireDirection = _mesh->GetSocketRotation(TEXT("Muzzle")).Vector();
	FHitResult hitResult;

	// 조준하고 있지 않을 경우 탄퍼짐
	if (!_owner->GetStateComponent()->IsAiming())
	{
		fireDirection = FMath::VRandCone(fireDirection, FMath::DegreesToRadians(_gunData._shakeAmount));
	}

	for (int i = 0; i < _pelletCount; i++)
	{
		FVector pelletDirection = FMath::VRandCone(fireDirection, _horizontalSpread * 0.001f, _verticalSpread * 0.001f);
		FVector end = muzzleLocation + pelletDirection * 10000.f;

		GetWorld()->LineTraceSingleByChannel(
			hitResult,
			muzzleLocation,
			end,
			ECC_Pawn);

		if (hitResult.bBlockingHit)
		{
			drawColor = FColor::Red;
			float distance = FVector::Dist(hitResult.TraceStart, hitResult.ImpactPoint);
			float finalDamage = CalculateDamage(distance / 100);

			if (ACharacterBase* character = Cast<ACharacterBase>(hitResult.GetActor()))
			{
				UGameplayStatics::ApplyDamage(character, finalDamage, _owner->GetController(), this, nullptr);
			}
		}

		FVector hitPoint = hitResult.bBlockingHit ? hitResult.ImpactPoint : hitResult.TraceEnd;
		DrawDebugLine(GetWorld(), hitResult.TraceStart, hitPoint, drawColor, false, 1.0f);
	}


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
