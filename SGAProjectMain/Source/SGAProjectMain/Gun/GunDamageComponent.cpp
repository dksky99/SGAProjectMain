// Fill out your copyright notice in the Description page of Project Settings.


#include "GunDamageComponent.h"
#include "GunBase.h"

// Sets default values for this component's properties
UGunDamageComponent::UGunDamageComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UGunDamageComponent::SetDamageData(const FGunData& gunData)
{
	_baseDamage = gunData._baseDamage;
	_vsDurableDamage = gunData._vsDurableDamage;

	_penetrateTrait = gunData._penetrateTrait;

	_falloff25 = gunData._falloff25;
	_falloff50 = gunData._falloff50;
	_falloff100 = gunData._falloff100;

	_shakeAmount = gunData._shakeAmount;
}


// Called when the game starts
void UGunDamageComponent::BeginPlay()
{
	Super::BeginPlay();

	_gun = Cast<AGunBase>(GetOwner()); // TODO) Comp가 gun을 들고 있어야 하는가?

}

void UGunDamageComponent::FireShot(FVector muzzleLocation, FRotator muzzleRotation, bool isAiming)
{
	FVector fireDirection = muzzleRotation.Vector();
	FVector fireLocation = muzzleLocation + fireDirection * 20.f;

	if (!isAiming)
	{
		fireDirection = FMath::VRandCone(fireDirection, FMath::DegreesToRadians(_shakeAmount));
	}

	FRotator fireRotation = fireDirection.Rotation();

	DoFireShot(fireLocation, fireDirection);
}

float UGunDamageComponent::CalculateDamage(float distance) // distance는 meter 단위
{
	if (distance <= 25.f) // 25m까지
	{
		float alpha = distance / 25.0f;
		float falloff = FMath::Lerp(0.0f, _falloff25, alpha);
		return _baseDamage * (1.0f - falloff);
	}
	else if (distance <= 50.f) // 50m까지
	{
		float alpha = (distance - 25.0f) / 25.0f;
		float falloff = FMath::Lerp(_falloff25, _falloff50, alpha);
		return _baseDamage * (1.0f - falloff);
	}
	else if (distance <= 100.f) // 100m까지
	{
		float alpha = (distance - 50.0f) / 50.0f;
		float falloff = FMath::Lerp(_falloff50, _falloff100, alpha);
		return _baseDamage * (1.0f - falloff);
	}
	else
	{
		// 50~100m 구간의 감속 기울기
		float perMeterFalloff = (_falloff100 - _falloff50) / 50.0f;

		// 100m 이후부터는 50~100m 구간의 감속 기울기 사용
		float extraFalloff = perMeterFalloff * ((distance - 100.f));
		float finalFalloff = _falloff100 + extraFalloff;

		return _baseDamage * (1.0f - finalFalloff);
	}
}

