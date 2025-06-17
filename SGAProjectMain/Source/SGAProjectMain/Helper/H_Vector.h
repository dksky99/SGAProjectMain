// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class SGAPROJECTMAIN_API H_Vector
{
public:
	UFUNCTION(BlueprintPure)
	static FVector MoveVectorUpward(FVector inValue, float addValue);
	UFUNCTION(BlueprintPure)
	static FVector MoveVectorDownward(FVector inValue, float subValue);
	UFUNCTION(BlueprintPure)
	static FVector MoveVectorForward(FVector inVector,FRotator inRotation, float addValue);
	UFUNCTION(BlueprintPure)
	static FVector MoveVectorBackward(FVector inVector,FRotator inRotation, float subValue);
	UFUNCTION(BlueprintPure)
	static FVector MoveVectorLeft(FVector inVector,FRotator inRotation, float subValue);
	UFUNCTION(BlueprintPure)
	static FVector MoveVectorRight(FVector inVector,FRotator inRotation, float addValue);
	UFUNCTION(BlueprintPure)
	static FRotator ReverseNormal(FVector inNormal);


};
