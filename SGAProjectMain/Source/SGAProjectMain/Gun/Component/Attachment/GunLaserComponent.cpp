// Fill out your copyright notice in the Description page of Project Settings.


#include "GunLaserComponent.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

UGunLaserComponent::UGunLaserComponent()
{
	/*static ConstructorHelpers::FObjectFinder<UNiagaraSystem> fireNS(TEXT("/Script/Niagara.NiagaraSystem'/Game/Graphics/Gun/Effect/NS_WeaponFire.NS_WeaponFire'"));
	if (fireNS.Succeeded())
	{
		_fireNS = fireNS.Object;
	}

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> shellEjectNS(TEXT("/Script/Niagara.NiagaraSystem'/Game/Graphics/Gun/Effect/NS_WeaponFire_ShellEject.NS_WeaponFire_ShellEject'"));
	if (shellEjectNS.Succeeded())
	{
		_shellEjectNS = shellEjectNS.Object;
	}*/
}

void UGunLaserComponent::InitializeAttachment(USkeletalMeshComponent* gunMesh)
{
	if (_laserNS)
	{
		_laserEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
			_laserNS,
			gunMesh,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}

	if (_laserImpactNS)
	{
		_laserImpactEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
			_laserImpactNS,
			gunMesh,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}
}

void UGunLaserComponent::OnAimChanged(bool isAiming)
{
	if (_laserEffect && _laserImpactEffect)
	{
		_laserEffect->SetVisibility(isAiming);
		_laserImpactEffect->SetVisibility(isAiming);
	}
}

void UGunLaserComponent::UpdateAttachment(USkeletalMeshComponent* gunMesh, FVector hitPoint)
{
	FVector start;

	if (gunMesh && gunMesh->DoesSocketExist(TEXT("LaserPoint")))
	{
		start = gunMesh->GetSocketLocation(TEXT("LaserPoint"));
	}
	else
	{
		start = GetOwner()->GetActorLocation();
	}

	FVector end = hitPoint;

	if (_laserEffect)
	{
		_laserEffect->SetVectorParameter("Beam Start", start);
		_laserEffect->SetVectorParameter("Beam End", end);
	}

	if (_laserImpactEffect)
		_laserImpactEffect->SetWorldLocation(end);
}