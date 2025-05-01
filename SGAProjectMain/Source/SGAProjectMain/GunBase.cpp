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
	
}

// Called every frame
void AGunBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FHitResult hitResult;
	FCollisionQueryParams params(NAME_None, false, this);

	FVector playerLocation = FVector::ZeroVector;

	auto player = GetWorld()->GetFirstPlayerController()->GetPawn();
	auto camera = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;

	if (player && camera)
	{
		FRotator cameraRotation = camera->GetCameraRotation();

		start = player->GetActorLocation() + cameraRotation.Vector() * 100;
		FVector end = start + cameraRotation.Vector() * 10000;
		bool bResult = GetWorld()->LineTraceSingleByChannel(
			OUT hitResult,
			start,
			end,
			ECC_Visibility,
			params);

		if (bResult)
		{
			_hitPoint = hitResult.Location;
			_isHit = true;
		}
		else
		{
			_hitPoint = end;
			_isHit = false;
		}
		
		if (!IsValid(_marker))
		{
			_marker = GetWorld()->SpawnActor<AImpactMarker>(_impactMarkerClass, _hitPoint, FRotator::ZeroRotator);
		}
		else
		{
			_marker->SetActorLocation(_hitPoint);
		}
	}
}

void AGunBase::StartFire()
{
	UE_LOG(LogTemp, Log, TEXT("STARTFIRE"));
	GetWorldTimerManager().SetTimer(_fireTimer, this, &AGunBase::Fire, _fireInterval, true, 0.0f);
}

void AGunBase::Fire()
{
	FColor drawColor = FColor::Green;

	if (_isHit)
	{
		drawColor = FColor::Red;
		//TODO
	}
	
	UE_LOG(LogTemp, Log, TEXT("FIRE"));
	DrawDebugLine(GetWorld(), start, _hitPoint, drawColor, false, 1.0f);
}

void AGunBase::StopFire()
{
	UE_LOG(LogTemp, Log, TEXT("STOPFIRE"));
	GetWorldTimerManager().ClearTimer(_fireTimer);
}

