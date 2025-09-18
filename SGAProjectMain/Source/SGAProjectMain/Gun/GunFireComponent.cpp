// Fill out your copyright notice in the Description page of Project Settings.


#include "GunFireComponent.h"

#include "GunBase.h"



// Sets default values for this component's properties
UGunFireComponent::UGunFireComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UGunFireComponent::SetFireModes(TArray<EFireMode> fireModes)
{
	_fireModes = fireModes;
	_curFireMode = _fireModes[0];
}


// Called when the game starts
void UGunFireComponent::BeginPlay()
{
	Super::BeginPlay();

	_gun = Cast<AGunBase>(GetOwner());
	// ...
	
}


// Called every frame
void UGunFireComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UGunFireComponent::StartFire()
{
	if (auto owner = _gun->GetOwnerCharacter())
		owner->GetStateComponent()->SetFiring(true);

	GetWorld()->GetTimerManager().ClearTimer(_fireTimer);

	float _fireInterval = _gun->GetGunData()._fireInterval;

	switch (_curFireMode)
	{
	case EFireMode::FireAuto: // 누르는 동안 발사
		GetWorld()->GetTimerManager().SetTimer(_fireTimer, this, &UGunFireComponent::TryFire, _fireInterval, true);
		break;

	case EFireMode::FireBoltAction: // 볼트액션
	case EFireMode::FireSemi: // 한 번만 발사
		TryFire();
		StopFire();
		break;
	case EFireMode::FireBurst: // 3발 발사
		_burstCount = 3;
		GetWorld()->GetTimerManager().SetTimer(_fireTimer, this, &UGunFireComponent::TryFire, _fireInterval, true);
	}
}

void UGunFireComponent::TryFire()
{
	// 쏘던 중 탄창, 약실 모두 비었음
	if (!_gun->CanFire())
	{
		StopFire();
		return;
	}

	if (_curFireMode == EFireMode::FireBoltAction)
	{
		if (!_canBoltFire) // 아직 볼트액션 쿨타임 도는 중
		{
			StopFire();
			return;
		}

		_canBoltFire = false;
		GetWorld()->GetTimerManager().SetTimer(_boltActionTimer, this, &UGunFireComponent::ResetBolt, _gun->GetGunData()._fireInterval, false);
	}

	if (_curFireMode == EFireMode::FireBurst)
	{
		if (_burstCount <= 0)	// 3발 다 쏘았음
		{
			StopFire();
			return;
		}

		_burstCount--;
	}

	if (_fireEvent.IsBound())
		_fireEvent.Broadcast(); // 총에서 Fire() 실행
}

void UGunFireComponent::StopFire()
{
	if (auto owner = _gun->GetOwnerCharacter())
		owner->GetStateComponent()->SetFiring(false);

	GetWorld()->GetTimerManager().ClearTimer(_fireTimer);
}

void UGunFireComponent::ChangeFireMode()
{
	if (_fireModes.Num() <= 1)
		return;

	_curFireIndex = (_curFireIndex + 1) % _fireModes.Num();
	_curFireMode = _fireModes[_curFireIndex];
}

