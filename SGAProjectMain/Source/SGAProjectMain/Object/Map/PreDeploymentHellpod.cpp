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
	if (!_isInteractable) return;

	if (!hellDiver)
		return;

	_isInteractable = false;

	if (!_preDeployFlow)
	{
		_preDeployFlow = NewObject<UPreDeploymentFlow>(this, _preDeployFlowClass);
	}

	auto player = Cast<APlayerCharacter>(hellDiver);

	_preDeployFlow->Initialize(player);
	_preDeployFlow->EnterFlow();
}

void APreDeploymentHellpod::ShowDefaultMark()
{
	Super::ShowDefaultMark();
	_interactionMark->SetVisibility(_isInteractable);
}

void APreDeploymentHellpod::ShowKeyButtonMark()
{
	Super::ShowKeyButtonMark();
	_interactionMark->SetVisibility(_isInteractable);
}

void APreDeploymentHellpod::SetInteractable(bool isInteractable)
{
	_isInteractable = isInteractable;
}