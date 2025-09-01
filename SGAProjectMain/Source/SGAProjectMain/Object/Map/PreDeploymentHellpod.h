// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Interactable.h"
#include "PreDeploymentHellpod.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API APreDeploymentHellpod : public AInteractable
{
	GENERATED_BODY()
	
public:
	APreDeploymentHellpod();

	virtual void Interact(class AHellDiver* hellDiver);

protected:
	UPROPERTY(EditAnywhere, Category = "Game/Mesh")
	TObjectPtr<USkeletalMeshComponent> _hellpodMesh;

	// √‚∞› »Â∏ß
	UPROPERTY()
	class UPreDeploymentFlow* _preDeployFlow;

	UPROPERTY(EditAnywhere, Category = "Game/Flow")
	TSubclassOf<UPreDeploymentFlow> _preDeployFlowClass;
};
