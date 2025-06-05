// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyStatComponent.h"

void UEnemyStatComponent::ChangeHp(float amount)
{
	Super::ChangeHp(amount);

	if (_hpChanged.IsBound())
		_hpChanged.Broadcast(_curHp / _maxHp);
}
