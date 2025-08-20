// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeploymentHellpod.h"

#include "Kismet/GameplayStatics.h"

APreDeploymentHellpod::APreDeploymentHellpod()
{
	if (_mesh)  // AItemBaseÀÇ StaticMesh »èÁ¦
	{
		_mesh->DestroyComponent();
		_mesh->SetHiddenInGame(true);
	}

	_hellpodMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMesh"));
	_hellpodMesh->SetGenerateOverlapEvents(true);
	RootComponent = _hellpodMesh;
}

void APreDeploymentHellpod::PickupItem(AHellDiver* player)
{
	if (!player)
		return;

	UGameplayStatics::OpenLevel(this, FName("FirstPersonMap"));
}