// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StratagemBase.h"
#include "StrikeStratagemBase.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AStrikeStratagemBase : public AStratagemBase
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stratagem")
	float _damage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stratagem")
	float _radius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	UParticleSystem* _effect;
};
