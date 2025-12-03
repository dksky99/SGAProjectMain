// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy_Spitter.h"
#include "Enemy_Spewer.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AEnemy_Spewer : public AEnemy_Spitter
{
	GENERATED_BODY()

public:

	AEnemy_Spewer(const FObjectInitializer& ObjectInitializer);



	virtual struct FUnitPartStat* GetHittedPartStat(EBodyPart part,const UPrimitiveComponent* OverlappedComponent, FVector hitLoc ) override;

	virtual bool CheckAbleTryNear(AActor* target) override;
	virtual bool TryNear(AActor* target)   override;

protected:

	virtual void PartInit() override;
protected:


};
