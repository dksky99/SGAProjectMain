// Fill out your copyright notice in the Description page of Project Settings.


#include "GunBase.h"

#include "Engine/DamageEvents.h"
#include "../UI/ImpactMarker.h"
#include "Blueprint/UserWidget.h"

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
	_curAmmo = _maxAmmo;
}

// Called every frame
void AGunBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	auto player = GetWorld()->GetFirstPlayerController()->GetPawn();
	auto camera = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
	if (!player || !camera) return;
	
	// 화면 중앙 방향
	FVector cameraLocation;
	FRotator cameraRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(cameraLocation, cameraRotation);
	
	FVector start = cameraLocation;

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
	
	if (!_isAiming)
		return;

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
	
	auto player = GetWorld()->GetFirstPlayerController()->GetPawn();
	auto camera = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
	if (!player || !camera) return;

	FRotator cameraRotation = camera->GetCameraRotation();
	FVector start = player->GetActorLocation() + cameraRotation.Vector() * 100;

	// 플레이어에게서 화면 중앙 방향으로
	FVector dir = (_hitPoint - start).GetSafeNormal();
	
	// 조준하지 않을 경우 탄퍼짐
	if (!_isAiming)
	{
		dir = FMath::VRandCone(dir, FMath::DegreesToRadians(15.0f));
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
		// TODO (데미지)
	}

	_curAmmo--;
	
	if (_ammoChanged.IsBound())
		_ammoChanged.Broadcast(_curAmmo, _maxAmmo);

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
		_ammoChanged.Broadcast(_curAmmo, _maxAmmo);
}

void AGunBase::Reload()
{
	_curAmmo = _maxAmmo;
	if (_ammoChanged.IsBound())
		_ammoChanged.Broadcast(_curAmmo, _maxAmmo);
}

