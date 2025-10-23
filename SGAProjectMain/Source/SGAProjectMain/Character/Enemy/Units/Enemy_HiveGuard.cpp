// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_HiveGuard.h"

AEnemy_HiveGuard::AEnemy_HiveGuard(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
}

bool AEnemy_HiveGuard::CheckAbleTryNear(AActor* target)
{
    return false;
}

bool AEnemy_HiveGuard::CheckAbleTryMiddle(AActor* target)
{
    return false;
}

bool AEnemy_HiveGuard::TryNear(AActor* target)
{
    return false;
}

bool AEnemy_HiveGuard::TryMiddle(AActor* target)
{
    return false;
}

bool AEnemy_HiveGuard::TryFar(AActor* target)
{
    return false;
}

void AEnemy_HiveGuard::StartGuard()
{
}

void AEnemy_HiveGuard::EndGuard()
{
}
