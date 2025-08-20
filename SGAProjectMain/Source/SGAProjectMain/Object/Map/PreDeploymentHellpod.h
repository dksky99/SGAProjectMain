// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Item/ItemBase.h"
#include "PreDeploymentHellpod.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API APreDeploymentHellpod : public AItemBase
{
	GENERATED_BODY()
	
public:
	APreDeploymentHellpod();

	virtual void PickupItem(class AHellDiver* player);

protected:
	UPROPERTY(EditAnywhere, Category = "Game/Mesh")
	TObjectPtr<USkeletalMeshComponent> _hellpodMesh;
};
