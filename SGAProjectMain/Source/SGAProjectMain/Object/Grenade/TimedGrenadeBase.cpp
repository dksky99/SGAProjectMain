// Fill out your copyright notice in the Description page of Project Settings.


#include "TimedGrenadeBase.h"
#include "Engine/EngineTypes.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void ATimedGrenadeBase::StartCookingGrenade()
{
	_cookedTime = GetWorld()->GetTimeSeconds();
}

void ATimedGrenadeBase::UpdateCookingGrenade()
{
	if (IsFuseTimeRemaining() || _isExploded)
		return;

	ExplodeGrenade();
}

void ATimedGrenadeBase::Throw(FVector direction)
{
	float remainingFuseTime = GetRemainingFuseTime();
	if (remainingFuseTime < 0)
		ExplodeGrenade();
	else
		GetWorldTimerManager().SetTimer(_explosionTimerHandle, this, &ATimedGrenadeBase::ExplodeGrenade, remainingFuseTime, false);

	Super::Throw(direction);
}

void ATimedGrenadeBase::ExplodeGrenade()
{
	if (IsFuseTimeRemaining() || _isExploded)
		return;

	_isExploded = true;

	GetWorldTimerManager().ClearTimer(_explosionTimerHandle);  // 타이머 제거

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

	DestroySelf();
}

bool ATimedGrenadeBase::IsFuseTimeRemaining()
{
	return(GetWorld()->GetTimeSeconds() - _cookedTime) < _totalFuseTime;
}

float ATimedGrenadeBase::GetRemainingFuseTime()
{
	return _totalFuseTime - (GetWorld()->GetTimeSeconds() - _cookedTime);
}
