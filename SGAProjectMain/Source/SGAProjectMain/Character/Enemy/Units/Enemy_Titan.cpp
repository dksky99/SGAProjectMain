// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_Titan.h"

AEnemy_Titan::AEnemy_Titan(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
}

bool AEnemy_Titan::CheckAbleTryNear(AActor* target)
{
    return false;
}

bool AEnemy_Titan::CheckAbleTryMiddle(AActor* target)
{
    return false;
}

bool AEnemy_Titan::TryNear(AActor* target)
{
    return false;
}

bool AEnemy_Titan::TryMiddle(AActor* target)
{
    return false;
}

bool AEnemy_Titan::TryFar(AActor* target)
{
    return false;
}
