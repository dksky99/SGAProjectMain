// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_Charger.h"

AEnemy_Charger::AEnemy_Charger(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
}

bool AEnemy_Charger::CheckAbleTryNear(AActor* target)
{
    return false;
}

bool AEnemy_Charger::CheckAbleTryMiddle(AActor* target)
{
    return false;
}

bool AEnemy_Charger::TryNear(AActor* target)
{
    return false;
}

bool AEnemy_Charger::TryMiddle(AActor* target)
{
    return false;
}

bool AEnemy_Charger::TryFar(AActor* target)
{
    return false;
}

void AEnemy_Charger::StartCharging()
{
}

void AEnemy_Charger::FinishCharging()
{
}

void AEnemy_Charger::CancelCharging()
{
}
