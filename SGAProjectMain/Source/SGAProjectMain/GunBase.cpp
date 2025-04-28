// Fill out your copyright notice in the Description page of Project Settings.


#include "GunBase.h"

#include "Engine/DamageEvents.h"

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

}

void AGunBase::StartFire()
{
	GetWorldTimerManager().SetTimer(_fireTimer, this, &AGunBase::Fire, _fireInterval, true, 0.0f);
}

void AGunBase::Fire()
{
	FHitResult hitResult;
	FCollisionQueryParams params(NAME_None, false, this);

	FVector start = GetActorLocation() + GetActorForwardVector() * 100;
	FVector end = start + GetActorForwardVector() * 10000;
	bool bResult = GetWorld()->LineTraceSingleByChannel(
		OUT hitResult,
		start,
		end,
		ECC_Visibility,
		params);

	FColor drawColor = FColor::Green;

	if (bResult)
	{
		drawColor = FColor::Red;
		// TODO
	}

	DrawDebugLine(GetWorld(), start, end, drawColor, false, 1.0f);
}

void AGunBase::StopFire()
{
	GetWorldTimerManager().ClearTimer(_fireTimer);
}

