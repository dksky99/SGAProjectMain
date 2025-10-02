// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../StatComponent.h"
#include "HellDiverStatComponent.generated.h"

/**
 * 
 */
//DECLARE_MULTICAST_DELEGATE_OneParam(FHpChanged, float);
DECLARE_MULTICAST_DELEGATE_TwoParams(FStaminaChanged, float, bool);

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

	//FHpChanged _hpChanged;
	//void ChangeHp(float amount) override;

	void ConsumeStamina(float deltaTime);
	void RecoverStamina(float deltaTime);
	void RecoverFullStamina() { _curStamina = _maxStamina; }
	float GetCurStamina() { return _curStamina; }
	bool IsMaxStamina() { return _curStamina == _maxStamina; }

	FStaminaChanged _staminaChanged;

protected:

	float _defaultSpeed = 500.0f;

	float _sprintSpeed = 700.0f;

	float _crouchSpeed = 300.0f;

	float _proneSpeed = 200.0f;

	float _power = 1000.0f;

	float _maxStamina = 100.0f;
	float _curStamina = 100.0f;


};
