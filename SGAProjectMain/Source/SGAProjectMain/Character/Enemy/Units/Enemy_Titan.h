// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy_Spewer.h"
#include "Enemy_Titan.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AEnemy_Titan : public AEnemy_Spewer
{
	GENERATED_BODY()

public:

	AEnemy_Titan(const FObjectInitializer& ObjectInitializer);




	virtual bool CheckAbleTryNear(AActor* target) override;
	virtual bool CheckAbleTryMiddle(AActor* target) override;
	virtual bool TryNear(AActor* target)   override;
	virtual bool TryMiddle(AActor* target) override;
	virtual bool TryFar(AActor* target)    override;


};
