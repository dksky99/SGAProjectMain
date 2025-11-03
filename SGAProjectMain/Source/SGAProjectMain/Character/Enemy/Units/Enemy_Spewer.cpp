// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_Spewer.h"

AEnemy_Spewer::AEnemy_Spewer(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	_errorDegree = 15.0f;
}

bool AEnemy_Spewer::CheckAbleTryNear(AActor* target)
{

	if (target == nullptr)
		return false;
	return true;
}

bool AEnemy_Spewer::TryNear(AActor* target)
{
	if (CheckAbleTryNear(target) == false)
		return false;
	if (AttackMelee())
		return true;


	return false;
}

