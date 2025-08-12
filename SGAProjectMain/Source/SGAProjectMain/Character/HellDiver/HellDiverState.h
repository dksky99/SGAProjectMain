// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HellDiverState.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UHellDiverState : public UObject
{
	GENERATED_BODY()
public:
	virtual void Move() { }
	virtual void Look() {}
	virtual void Fire() {}
	virtual void Rolliing() {}
	virtual void TryPakour() {}
	virtual void Standing() {}
	virtual void Crouching() {}
	virtual void Sprinting() {}
	virtual void Proning() {}


protected:
};
