// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "Backpack.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API ABackpack : public AItemBase
{
	GENERATED_BODY()

public:
	ABackpack();

	virtual void PickupItem(class AHellDiver* player);

	USkeletalMeshComponent* GetMesh() { return _skeletalMesh; }

protected:
	UPROPERTY(EditAnywhere, Category = "Game/Item")
	USkeletalMeshComponent* _skeletalMesh;
};
