// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy_Standard.h"
#include "Enemy_Warrior.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AEnemy_Warrior : public AEnemy_Standard
{
	GENERATED_BODY()

public:

	AEnemy_Warrior(const FObjectInitializer& ObjectInitializer);




	virtual bool CheckAbleTryMiddle(AActor* target) override;
	virtual bool TryMiddle(AActor* target) override;
};
