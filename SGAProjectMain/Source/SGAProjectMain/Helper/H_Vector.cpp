// Fill out your copyright notice in the Description page of Project Settings.


#include "H_Vector.h"
#include "Kismet/KismetMathLibrary.h"


FVector H_Vector::MoveVectorUpward(FVector inValue, float addValue)
{
	FVector Result = inValue;
	Result.Z += addValue;
	return Result;
}

FVector H_Vector::MoveVectorDownward(FVector inValue, float subValue)
{
	FVector Result = inValue;
	Result.Z -= subValue;
	return Result;
}

FVector H_Vector::MoveVectorForward(FVector inVector, FRotator inRotation, float addValue)
{

	FVector temp = UKismetMathLibrary::GetForwardVector(inRotation);
	temp*= addValue;
	FVector result = inVector + temp;
	return result;
}

FVector H_Vector::MoveVectorBackward(FVector inVector, FRotator inRotation, float subValue)
{
	FVector temp = UKismetMathLibrary::GetForwardVector(inRotation);
	temp *= subValue;
	FVector result = inVector - temp;
	return result;
}

FVector H_Vector::MoveVectorLeft(FVector inVector, FRotator inRotation, float subValue)
{
	FVector temp = UKismetMathLibrary::GetRightVector(inRotation);
	temp *= subValue;
	FVector result = inVector - temp;
	return result;
}

FVector H_Vector::MoveVectorRight(FVector inVector, FRotator inRotation, float addValue)
{
	FVector temp = UKismetMathLibrary::GetRightVector(inRotation);
	
	temp *= addValue;
	FVector result = inVector + temp;
	return result;
}

FRotator H_Vector::ReverseNormal(FVector inNormal)
{
	FRotator temp = UKismetMathLibrary::MakeRotFromX(inNormal);
	
	return UKismetMathLibrary::NormalizedDeltaRotator(temp, FRotator(0, 0, 180));
}
