// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeploymentHellpod.h"

#include "Kismet/GameplayStatics.h"

APreDeploymentHellpod::APreDeploymentHellpod()
{
	_hellpodMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMesh"));
	_hellpodMesh->SetGenerateOverlapEvents(true);
	RootComponent = _hellpodMesh;
}

void APreDeploymentHellpod::Interact(AHellDiver* player)
{
	if (!player)
		return;

	UGameplayStatics::OpenLevel(this, FName("FirstPersonMap"));
}