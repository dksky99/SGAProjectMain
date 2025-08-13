// Fill out your copyright notice in the Description page of Project Settings.


#include "TimedGrenadeBase.h"

#include "SGAProjectMain/SGAProjectMain.h"
#include "../ExplosionComponent.h"

#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"
#include "TimerManager.h"

ATimedGrenadeBase::ATimedGrenadeBase()
{
	_explosionComponent = CreateDefaultSubobject<UExplosionComponent>(TEXT("ExplosionComponent"));
}

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

	_explosionComponent->Explode();

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
