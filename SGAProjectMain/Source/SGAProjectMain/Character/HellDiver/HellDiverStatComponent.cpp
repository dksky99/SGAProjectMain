// Fill out your copyright notice in the Description page of Project Settings.


#include "HellDiverStatComponent.h"

#include "HellDiver.h"

void UHellDiverStatComponent::ChangeHp(float amount)
{
	Super::ChangeHp(amount);

	if (_hpChanged.IsBound())
		_hpChanged.Broadcast(_curHp / _maxHp);
}

void UHellDiverStatComponent::ConsumeStamina(float deltaTime)
{
	_curStamina -= deltaTime * 50.f;//* 5.f;

	if (_curStamina <= 0.f)
	{
		_curStamina = 0.f;
	}

	if (_staminaChanged.IsBound())
		_staminaChanged.Broadcast(_curStamina / _maxStamina, false);
}

void UHellDiverStatComponent::RecoverStamina(float deltaTime)
{
	_curStamina += deltaTime * 70.f; //* 7.f;

	if (_curStamina > _maxStamina)
		_curStamina = _maxStamina;

	if (_staminaChanged.IsBound())
		_staminaChanged.Broadcast(_curStamina / _maxStamina, true);
}
