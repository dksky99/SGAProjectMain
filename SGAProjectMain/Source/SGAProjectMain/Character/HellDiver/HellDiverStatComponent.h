// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../StatComponent.h"
#include "HellDiverStatComponent.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UHellDiverStatComponent : public UStatComponent
{
	GENERATED_BODY()
public:




protected:


	float _sprintSpeed = 400.0f;

	float _crouchSpeed = 150.0f;

	float _proneSpeed = 100.0f;
};
