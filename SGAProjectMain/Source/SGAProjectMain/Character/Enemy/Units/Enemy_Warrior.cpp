// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_Warrior.h"

AEnemy_Warrior::AEnemy_Warrior(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
}

bool AEnemy_Warrior::CheckAbleTryNear(AActor* target)
{
    return false;
}

bool AEnemy_Warrior::CheckAbleTryMiddle(AActor* target)
{
    return false;
}

bool AEnemy_Warrior::TryNear(AActor* target)
{
    return false;
}

bool AEnemy_Warrior::TryMiddle(AActor* target)
{
    return false;
}

bool AEnemy_Warrior::TryFar(AActor* target)
{
    return false;
}
