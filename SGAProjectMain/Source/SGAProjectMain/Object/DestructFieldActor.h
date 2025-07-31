// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Field/FieldSystemActor.h"
#include "DestructFieldActor.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API ADestructFieldActor : public AFieldSystemActor
{
	GENERATED_BODY()
	
public:
	void ActivateField(const FVector& Position, float Radius = 5000.f, float Strength = 10000.f);
};
