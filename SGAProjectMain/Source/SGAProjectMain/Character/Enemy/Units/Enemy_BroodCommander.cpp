// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_BroodCommander.h"

AEnemy_BroodCommander::AEnemy_BroodCommander(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
}

bool AEnemy_BroodCommander::CheckAbleTryNear(AActor* target)
{
    return false;
}

bool AEnemy_BroodCommander::CheckAbleTryMiddle(AActor* target)
{
    return false;
}

bool AEnemy_BroodCommander::TryNear(AActor* target)
{
    return false;
}

bool AEnemy_BroodCommander::TryMiddle(AActor* target)
{
    return false;
}

bool AEnemy_BroodCommander::TryFar(AActor* target)
{
    return false;
}

void AEnemy_BroodCommander::CallWarrior()
{
}
