// Fill out your copyright notice in the Description page of Project Settings.


#include "GunBase.h"

#include "Engine/DamageEvents.h"
#include "ImpactMarker.h"

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
	
	_curAmmo = _maxAmmo;
}

// Called every frame
void AGunBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	auto player = GetWorld()->GetFirstPlayerController()->GetPawn();
	auto camera = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
	if (!player || !camera) return;
	
	FRotator cameraRotation = camera->GetCameraRotation();
	start = player->GetActorLocation() + cameraRotation.Vector() * 100;

	if (!_isAiming)
		return;

	// 조준 시 라인트레이스 이용하여 마커 위치 계산
	FHitResult hitResult;
	FCollisionQueryParams params(NAME_None, false, this);

	FVector end = start + cameraRotation.Vector() * 10000;
	bool bResult = GetWorld()->LineTraceSingleByChannel(
		OUT hitResult,
		start,
		end,
		ECC_Visibility,
		params);

	if (bResult)
		_hitPoint = hitResult.Location;
	else
		_hitPoint = end;
	
	if (!IsValid(_marker))
		_marker = GetWorld()->SpawnActor<AImpactMarker>(_impactMarkerClass, _hitPoint, FRotator::ZeroRotator);
	else
		_marker->SetActorLocation(_hitPoint);
	
	UE_LOG(LogTemp, Log, TEXT("Tick"));
}

void AGunBase::StartFire()
{
	UE_LOG(LogTemp, Log, TEXT("STARTFIRE"));
	GetWorldTimerManager().SetTimer(_fireTimer, this, &AGunBase::Fire, _fireInterval, true, 0.0f);
}

void AGunBase::Fire()
{
	FColor drawColor = FColor::Green;
	
	// 탄약이 없을 경우
	if (_curAmmo <= 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Mag Empty"));
		return;
	}

	// 조준하지 않을 경우 탄퍼짐
	if (!_isAiming)
	{
		auto player = GetWorld()->GetFirstPlayerController()->GetPawn();
		auto camera = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
		if (!player || !camera) return;

		FRotator cameraRotation = camera->GetCameraRotation();
		FVector dir = cameraRotation.Vector();

		// 분산각 15도
		dir = FMath::VRandCone(dir, FMath::DegreesToRadians(15.0f));
		_hitPoint = player->GetActorLocation() + dir * 10000.0f;
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
		// TODO (데미지)
	}

	_curAmmo--;
	UE_LOG(LogTemp, Log, TEXT("FIRE"));
	DrawDebugLine(GetWorld(), start, _hitPoint, drawColor, false, 1.0f);
}

void AGunBase::StopFire()
{
	UE_LOG(LogTemp, Log, TEXT("STOPFIRE"));
	GetWorldTimerManager().ClearTimer(_fireTimer);
}

void AGunBase::StartAiming()
{
	_isAiming = true;

	if (_marker)
		_marker->SetActorHiddenInGame(false);

	UE_LOG(LogTemp, Log, TEXT("STARTAIMING"));
}

void AGunBase::StopAiming()
{
	_isAiming = false;

	if (_marker)
		_marker->SetActorHiddenInGame(true);

	UE_LOG(LogTemp, Log, TEXT("STOPAIMING"));
}

void AGunBase::Reload()
{
	_curAmmo = _maxAmmo;
}

