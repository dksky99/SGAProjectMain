// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Enemy.h"
#include "Enemy_Charger.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AEnemy_Charger : public AEnemy
{
	GENERATED_BODY()

public:

	AEnemy_Charger(const FObjectInitializer& ObjectInitializer);




	virtual bool CheckAbleTryNear(AActor* target) override;
	virtual bool CheckAbleTryMiddle(AActor* target) override;
	virtual bool TryNear(AActor* target)   override;
	virtual bool TryMiddle(AActor* target) override;
	virtual bool TryFar(AActor* target)    override;

	void StartCharging();

	void FinishCharging();

	void CancelCharging();

};
