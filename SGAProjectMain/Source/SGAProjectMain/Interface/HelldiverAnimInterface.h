// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HelldiverAnimInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHelldiverAnimInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SGAPROJECTMAIN_API IHelldiverAnimInterface 
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual void SetWeaponOffset(FTransform offset, bool useAim ) = 0;
	virtual bool SetLeftHandLocation(FVector inHandLocation) = 0;

};
