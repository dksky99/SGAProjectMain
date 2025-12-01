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

	virtual void AcidbagDestroyed() override;

	virtual struct FUnitPartStat* GetHittedPartStat(EBodyPart part, const UPrimitiveComponent* OverlappedComponent, FVector hitLoc) override;

	virtual void ResetUnit() override;
protected:

	UFUNCTION()
	void Titan_Bleeding();

	virtual void PartInit() override;
protected:

	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Ability/Bleeding", meta = (AllowPrivateAccess = "true"))
	float _additiveHP = 2000;


	bool _isBleeding = false;


};
