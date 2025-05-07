// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeBase.h"
#include "Engine/EngineTypes.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/OverlapResult.h"
#include "../Character/PlayerCharacter.h"

// Sets default values
AGrenadeBase::AGrenadeBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGrenadeBase::BeginPlay()
{
	Super::BeginPlay();

	_owner = Cast<APlayerCharacter>(GetOwner());
	
}

// Called every frame
void AGrenadeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGrenadeBase::StartCookingGrenade()
{
	_cookedTime = GetWorld()->GetTimeSeconds();
}

void AGrenadeBase::UpdateCookingGrenade()
{
	if ((GetWorld()->GetTimeSeconds() - _cookedTime) < _totalFuseTime)
		return;

	GetWorldTimerManager().ClearTimer(_explosionTimerHandle);  // 타이머 제거
	ExplodeGrenade();
}

void AGrenadeBase::ThrowGrenade()
{
	float remainingFuseTime = _totalFuseTime - (GetWorld()->GetTimeSeconds() - _cookedTime);
	GetWorldTimerManager().SetTimer(_explosionTimerHandle, this, &AGrenadeBase::ExplodeGrenade, remainingFuseTime, false);

	// 던지기 구현
}

void AGrenadeBase::ExplodeGrenade()
{
	if ((GetWorld()->GetTimeSeconds() - _cookedTime) < _totalFuseTime)
		return; // 아직 터질 시간이 아님

	TArray<FOverlapResult> overlaps;
	FCollisionQueryParams params(NAME_None, false, this);

	bool bResult = GetWorld()->OverlapMultiByChannel(
		overlaps,
		GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(_explosionRadius),
		params
	);

	if (bResult)
	{
		for (auto& overlap : overlaps)
		{
			AActor* hitActor = overlap.GetActor();
			if (hitActor)
			{
				UGameplayStatics::ApplyDamage(
					hitActor,
					_explosionDamage,
					GetInstigatorController(),
					this,
					nullptr
				);
			}
		}
	}

	// 이펙트 재생
	if (_explosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), _explosionEffect, GetActorLocation());
	}

	// 폭발 후 수류탄 액터 제거
	Destroy();
}

