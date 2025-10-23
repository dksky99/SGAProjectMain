// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_Hunter.h"

AEnemy_Hunter::AEnemy_Hunter(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
}

bool AEnemy_Hunter::CheckAbleTryNear(AActor* target)
{
    return false;
}

bool AEnemy_Hunter::CheckAbleTryMiddle(AActor* target)
{
    return false;
}

bool AEnemy_Hunter::TryNear(AActor* target)
{
    return false;
}

bool AEnemy_Hunter::TryMiddle(AActor* target)
{
    return false;
}

bool AEnemy_Hunter::TryFar(AActor* target)
{
    return false;
}

void AEnemy_Hunter::TryDodge()
{
}
