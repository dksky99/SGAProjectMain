// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_Spewer.h"

AEnemy_Spewer::AEnemy_Spewer(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
}

bool AEnemy_Spewer::CheckAbleTryNear(AActor* target)
{
    return false;
}

bool AEnemy_Spewer::CheckAbleTryMiddle(AActor* target)
{
    return false;
}

bool AEnemy_Spewer::TryNear(AActor* target)
{
    return false;
}

bool AEnemy_Spewer::TryMiddle(AActor* target)
{
    return false;
}

bool AEnemy_Spewer::TryFar(AActor* target)
{
    return false;
}
