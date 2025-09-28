// Fill out your copyright notice in the Description page of Project Settings.


#include "GunTacticalLightComponent.h"

#include "Components/SpotLightComponent.h"

void UGunTacticalLightComponent::InitializeAttachment(USkeletalMeshComponent* gunMesh)
{
    _lightComp = NewObject<USpotLightComponent>(GetOwner(), TEXT("TacticalLight"));
    _lightComp->RegisterComponent();
	_lightComp->AttachToComponent(gunMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("TacticalLight"));
	_lightComp->SetVisibility(false);

    _curLightMode = _lightModes[0];
}

void UGunTacticalLightComponent::ActivateAttachment(bool isActive, bool isAiming)
{
	if (isActive)
		OnAimChanged(isAiming);
	else
		_lightComp->SetVisibility(isActive);
}

void UGunTacticalLightComponent::OnAimChanged(bool isAiming)
{
	switch (_curLightMode)
	{
	case ETacticalLightMode::LightOn:
		_lightComp->SetVisibility(true);
		break;

	case ETacticalLightMode::LightOff:
		_lightComp->SetVisibility(false);
		break;

	case ETacticalLightMode::LightAuto:
		_lightComp->SetVisibility(isAiming);
		break;
	}
}

void UGunTacticalLightComponent::ChangeTacticalLightMode(bool isAiming)
{
	_curLightIndex = (_curLightIndex + 1) % _lightModes.Num();
	_curLightMode = _lightModes[_curLightIndex];

	OnAimChanged(isAiming);
}
