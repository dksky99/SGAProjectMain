// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../StatComponent.h"
#include "HellDiverStatComponent.generated.h"

/**
 * 
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FHpChanged, float);

UCLASS()
class SGAPROJECTMAIN_API UHellDiverStatComponent : public UStatComponent
{
	GENERATED_BODY()
public:


	float GetDefaultSpeed() { return _defaultSpeed; }
	float GetSprintSpeed() { return _sprintSpeed; }
	float GetCrouchSpeed() { return _crouchSpeed; }
	float GetProneSpeed() { return _proneSpeed; }
	float GetPower() { return _power; }

	FHpChanged _hpChanged;


protected:

	float _defaultSpeed = 500.0f;

	float _sprintSpeed = 700.0f;

	float _crouchSpeed = 300.0f;

	float _proneSpeed = 200.0f;

	float _power = 1000.0f;
};
