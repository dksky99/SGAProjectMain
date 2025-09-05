// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeploymentHellpod.h"

#include "../../Game/PreDeployment/PreDeploymentFlow.h"
#include "../../Character/PlayerCharacter.h"

APreDeploymentHellpod::APreDeploymentHellpod()
{
	_hellpodMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	_hellpodMesh->SetGenerateOverlapEvents(true);
	RootComponent = _hellpodMesh;
	_interactionMark->SetupAttachment(RootComponent);
	_mesh->SetupAttachment(RootComponent);
}

void APreDeploymentHellpod::Interact(AHellDiver* hellDiver)
{
	if (!hellDiver)
		return;

	if (!_preDeployFlow)
	{
		_preDeployFlow = NewObject<UPreDeploymentFlow>(this);
	}

	auto player = Cast<APlayerCharacter>(hellDiver);

	_preDeployFlow->Initialize(player);
	_preDeployFlow->EnterFlow();
}