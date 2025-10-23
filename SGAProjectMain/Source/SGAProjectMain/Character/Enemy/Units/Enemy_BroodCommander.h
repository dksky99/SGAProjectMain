// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy_Warrior.h"
#include "Enemy_BroodCommander.generated.h"

/**
 *  1. 병력소환, 
 */
UCLASS()
class SGAPROJECTMAIN_API AEnemy_BroodCommander : public AEnemy_Warrior
{
	GENERATED_BODY()

public:

	AEnemy_BroodCommander(const FObjectInitializer& ObjectInitializer);




	virtual bool CheckAbleTryNear(AActor* target) override;
	virtual bool CheckAbleTryMiddle(AActor* target) override;
	virtual bool TryNear(AActor* target)   override;
	virtual bool TryMiddle(AActor* target) override;
	virtual bool TryFar(AActor* target)    override;

	void CallWarrior();
};
