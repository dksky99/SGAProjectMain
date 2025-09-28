// Fill out your copyright notice in the Description page of Project Settings.


#include "GunScopeComponent.h"

void UGunScopeComponent::InitializeAttachment(USkeletalMeshComponent* gunMesh)
{
	_curScopeMode = _scopeModes[0];
}

void UGunScopeComponent::OnAimChanged(bool isAiming)
{
}

void UGunScopeComponent::ChangeScopeMode()
{
	_curScopeIndex = (_curScopeIndex + 1) % _scopeModes.Num();
	_curScopeMode = _scopeModes[_curScopeIndex];
}
