// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunDamageComponent.h"
#include "HitscanDamageComponent.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UHitscanDamageComponent : public UGunDamageComponent
{
	GENERATED_BODY()

protected:
	virtual void DoFireShot(FVector fireLocation, FVector fireDirection);
};
