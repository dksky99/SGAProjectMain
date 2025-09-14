// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UnitAttackDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UUnitAttackDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UAnimMontage* Motion;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Attack;


	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> ActiveColliders;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanMove;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UParticleSystem* Effect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector EffectOffset;
};
