// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "../Object/CDamageType.h"
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
	int32 Attack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 DurabilityAttack;


	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 DemolitionAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 PenetrationLevel; // 관통력 


	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Stagger = 0; // 비틀거림 유발.

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 PushForce = 0; // 밀치기 

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> ActiveColliders;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf< UCDamageType> DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Interval;




	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanMove;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UParticleSystem* Effect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector EffectOffset;
};
