// Fill out your copyright notice in the Description page of Project Settings.


#include "GunBase.h"

#include "Engine/DamageEvents.h"
#include "../UI/ImpactMarker.h"
#include "Blueprint/UserWidget.h"

#include "../Character/HellDiver/HellDiver.h"
#include "../Character/HellDiver/HellDiverStateComponent.h"

// Sets default values
AGunBase::AGunBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGunBase::BeginPlay()
{
	Super::BeginPlay();

	if (_crosshairClass)
	{
		_crosshair = CreateWidget<UUserWidget>(GetWorld(), _crosshairClass);

		if (_crosshair)
		{
			_crosshair->AddToViewport();
			_crosshair->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	_curAmmo = _gunData._maxAmmo;
	_maxVerticalRecoil = _gunData._verticalRecoil;
	_maxHorizontalRecoil = _gunData._horizontalRecoil;
}

// Called every frame
void AGunBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	auto player = GetWorld()->GetFirstPlayerController()->GetPawn();
	auto camera = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
	if (!player || !camera) return;
	
	TickRecoil(DeltaTime);
	_hitPoint = CalculateHitPoint();

	if (!_isAiming)
		return;

	if (!IsValid(_marker))
		_marker = GetWorld()->SpawnActor<AImpactMarker>(_impactMarkerClass, _hitPoint, FRotator::ZeroRotator);
	else
		_marker->SetActorLocation(_hitPoint);
}

void AGunBase::StartFire()
{
	if (AHellDiver* owner = Cast<AHellDiver>(GetOwner()))
	{
		owner->GetStateComponent()->SetFiring(true);
	}

	GetWorldTimerManager().ClearTimer(_fireTimer);

	switch (_fireMode)
	{
	case EFireMode::Auto: // 누르는 동안 발사
		GetWorldTimerManager().SetTimer(_fireTimer, this, &AGunBase::Fire, _gunData._fireInterval, true, 0.0f);
		break;
	case EFireMode::Semi: // 한 번만 발사
		Fire();
		StopFire();
		break;
	case EFireMode::Burst: // 3발 발사
		_burstCount = 3;
		GetWorld()->GetTimerManager().SetTimer(_fireTimer, this, &AGunBase::Fire, _gunData._fireInterval, true);
	}
}

void AGunBase::Fire()
{
	FColor drawColor = FColor::Green;
	
	// 탄약이 없을 경우
	if (_curAmmo <= 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Mag Empty"));
		StopFire();
		return;
	}
	
	auto player = GetWorld()->GetFirstPlayerController()->GetPawn();
	auto camera = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
	if (!player || !camera) return;

	if (_fireMode == EFireMode::Burst)
	{
		if (_burstCount <= 0)
		{
			StopFire();
			return;
		}
		_burstCount--;
	}

	ApplyFireRecoil();
	_hitPoint = CalculateHitPoint();

	FRotator cameraRotation = camera->GetCameraRotation();
	FVector start = player->GetActorLocation() + cameraRotation.Vector() * 100;

	// 플레이어에게서 화면 중앙 방향으로
	FVector dir = (_hitPoint - start).GetSafeNormal();
	
	// 조준하지 않을 경우 탄퍼짐
	if (!_isAiming)
	{
		dir = FMath::VRandCone(dir, FMath::DegreesToRadians(_gunData._recoil));
		_hitPoint = start + dir * 10000.0f; 
	}

	FHitResult hitResult;
	FCollisionQueryParams params(NAME_None, false, this);

	bool bResult = GetWorld()->LineTraceSingleByChannel(
		OUT hitResult,
		start,
		_hitPoint,
		ECC_Visibility,
		params);

	if (bResult)
	{
		drawColor = FColor::Red;
		float distance = FVector::Dist(start, hitResult.ImpactPoint);
		float finalDamage = CalculateDamage(distance / 100);
		UE_LOG(LogTemp, Log, TEXT("Final Damage: %f"), finalDamage);
		// TODO (데미지)
	}

	_curAmmo--;
	
	if (_ammoChanged.IsBound())
		_ammoChanged.Broadcast(_curAmmo, _gunData._maxAmmo);

	DrawDebugLine(GetWorld(), start, _hitPoint, drawColor, false, 1.0f);
}

void AGunBase::StopFire()
{
	if (AHellDiver* owner = Cast<AHellDiver>(GetOwner()))
	{
		owner->GetStateComponent()->SetFiring(false);
	}

	GetWorldTimerManager().ClearTimer(_fireTimer);
}

void AGunBase::StartAiming()
{
	_isAiming = true;

	if (_marker)
		_marker->SetActorHiddenInGame(false);
	if (_crosshair)
		_crosshair->SetVisibility(ESlateVisibility::Visible);

	UE_LOG(LogTemp, Log, TEXT("STARTAIMING"));
}

void AGunBase::StopAiming()
{
	_isAiming = false;

	if (_marker)
		_marker->SetActorHiddenInGame(true);
	if (_crosshair)
		_crosshair->SetVisibility(ESlateVisibility::Hidden);

	UE_LOG(LogTemp, Log, TEXT("STOPAIMING"));
}

void AGunBase::UpdateGun()
{
	if (_ammoChanged.IsBound())
		_ammoChanged.Broadcast(_curAmmo, _gunData._maxAmmo);
}

void AGunBase::Reload()
{
	_curAmmo = _gunData._maxAmmo;
	if (_ammoChanged.IsBound())
		_ammoChanged.Broadcast(_curAmmo, _gunData._maxAmmo);
}

float AGunBase::CalculateDamage(float distance)
{
	if (distance <= 25.0f)
	{
		float Alpha = distance / 25.0f;
		float Falloff = FMath::Lerp(0.0f, _gunData._falloff25, Alpha);
		return _gunData._baseDamage * (1.0f - Falloff);
	}
	else if (distance <= 50.0f)
	{
		float Alpha = (distance - 25.0f) / 25.0f;
		float Falloff = FMath::Lerp(_gunData._falloff25, _gunData._falloff50, Alpha);
		return _gunData._baseDamage * (1.0f - Falloff);
	}
	else if (distance <= 100.0f)
	{
		float Alpha = (distance - 50.0f) / 50.0f;
		float Falloff = FMath::Lerp(_gunData._falloff50, _gunData._falloff100, Alpha);
		return _gunData._baseDamage * (1.0f - Falloff);
	}
	else
	{
		return _gunData._baseDamage * (1.0f - _gunData._falloff100);
	}
}

void AGunBase::TickRecoil(float DeltaTime)
{
	float recoilMultiplier = GetRecoilMultiplier();

	static float RecoilTime = 0.f;
	RecoilTime += DeltaTime * 2.f * recoilMultiplier;

	// 임시값
	float amplitudePitch = 0.18f;
	float amplitudeYaw = 0.04f;

	_recoilOffset.Pitch += FMath::Sin(RecoilTime) * amplitudePitch * recoilMultiplier;
	_recoilOffset.Yaw += FMath::Cos(RecoilTime / 2) * amplitudeYaw * recoilMultiplier;

	_recoilOffset = FMath::RInterpTo(_recoilOffset, FRotator::ZeroRotator, DeltaTime, 4.f);
}

void AGunBase::ApplyFireRecoil()
{
	float recoilMultiplier = GetRecoilMultiplier();

	// 수직 반동 적용
	float vertical = FMath::RandRange(0.9f, 1.1f) * _gunData._verticalRecoil / 5.f;
	_recoilOffset.Pitch += vertical;

	_recoilOffset.Pitch = FMath::Clamp(_recoilOffset.Pitch, 0.f, _maxVerticalRecoil) * recoilMultiplier;

	// 수평 반동 적용
	float horizontal = FMath::RandRange(-1.f, 1.f) * _gunData._horizontalRecoil / 3.f;

	// 수직 반동이 최대에 도달하면 흔들림 적용
	if (_recoilOffset.Pitch >= _maxVerticalRecoil)
	{
		horizontal += FMath::RandRange(-1.f, 1.f) * _gunData._shakeAmount;
	}
	_recoilOffset.Yaw += horizontal;
	
	_recoilOffset.Yaw = FMath::Clamp(_recoilOffset.Yaw, -_maxHorizontalRecoil, _maxHorizontalRecoil) * recoilMultiplier;
}

float AGunBase::GetRecoilMultiplier()
{
	float base = 1.f;

	if (AHellDiver* owner = Cast<AHellDiver>(GetOwner()))
	{
		FVector velocity = owner->GetVelocity();

		// 움직이고 있지 않다면
		if (velocity.SizeSquared() < FMath::Square(10.f))
		{
			switch (owner->GetStateComponent()->GetCharacterState())
			{
			case ECharacterState::Standing:
				base = 1.f;
				break;
			case ECharacterState::Crouching:
				base = 0.6f;
				break;
			case ECharacterState::Proning:
				base = 0.4f;
				break;
			}
		}
		// 움직이고 있다면
		else
		{
			switch (owner->GetStateComponent()->GetCharacterState())
			{
			case ECharacterState::Standing:
				base = 1.5f;
				break;
			case ECharacterState::Crouching:
				base = 1.f;
				break;
			}
		}
	}
	return base;
}


FVector AGunBase::CalculateHitPoint()
{
	// 화면 중앙 방향
	FVector cameraLocation;
	FRotator cameraRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(cameraLocation, cameraRotation);

	FVector start = cameraLocation;

	// 조준 시 라인트레이스 이용하여 마커 위치 계산
	FHitResult hitResult;
	FCollisionQueryParams params(NAME_None, false, this);

	FVector end = start + (cameraRotation + _recoilOffset).Vector() * 10000;
	bool bResult = GetWorld()->LineTraceSingleByChannel(
		OUT hitResult,
		start,
		end,
		ECC_Visibility,
		params);

	if (bResult)
		return hitResult.Location;
	else
		return end;
}

void AGunBase::EnterGunSettingMode()
{
	// TODO - UI
}

void AGunBase::ExitGunSettingMode()
{
	// TODO -UI
}

void AGunBase::ChangeFireMode()
{
	if (_gunData._fireModes.Num() <= 1)
		return;

	_fireIndex = (_fireIndex + 1) % _gunData._fireModes.Num();
	_fireMode = _gunData._fireModes[_fireIndex];

	UE_LOG(LogTemp, Log, TEXT("Fire Mode Change"));
}

