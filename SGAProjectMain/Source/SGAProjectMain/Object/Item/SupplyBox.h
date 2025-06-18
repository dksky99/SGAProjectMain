// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "SupplyBox.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API ASupplyBox : public AItemBase
{
	GENERATED_BODY()
	

public:
	virtual void PickupItem(AHellDiver* player) override;
};
