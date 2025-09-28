// Fill out your copyright notice in the Description page of Project Settings.


#include "GunEffectComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

// Sets default values for this component's properties
UGunEffectComponent::UGunEffectComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> fireNS(TEXT("/Script/Niagara.NiagaraSystem'/Game/Graphics/Gun/Effect/NS_WeaponFire.NS_WeaponFire'"));
	if (fireNS.Succeeded())
	{
		_fireNS = fireNS.Object;
	}

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> shellEjectNS(TEXT("/Script/Niagara.NiagaraSystem'/Game/Graphics/Gun/Effect/NS_WeaponFire_ShellEject.NS_WeaponFire_ShellEject'"));
	if (shellEjectNS.Succeeded())
	{
		_shellEjectNS = shellEjectNS.Object;
	}
}


// Called when the game starts
void UGunEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UGunEffectComponent::InitializeEffect(USkeletalMeshComponent* gunMesh)
{
	if (_fireNS)
	{
		_fireEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
			_fireNS,
			gunMesh,
			TEXT("Muzzle"),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			false
		);
		_fireEffect->SetAutoDestroy(false);
		_fireEffect->Deactivate();
	}

	if (_shellEjectNS)
	{
		_shellEjectEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
			_shellEjectNS,
			gunMesh,
			TEXT("ShellEject"),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			false
		);
		_shellEjectEffect->SetAutoDestroy(false);
		_shellEjectEffect->Deactivate();
	}
}

void UGunEffectComponent::PlayFireEffect()
{
	if (_fireEffect)
	{
		_fireEffect->Activate(true);
	}

	if (_shellEjectEffect)
	{
		_shellEjectEffect->Activate(true);
	}
}

