// Fill out your copyright notice in the Description page of Project Settings.


#include "H_Rotator.h"


FVector H_Rotator::ProjectVectorOnPlane(const FVector& vector, const FVector& planeNormal)
{
	FVector normal = planeNormal.GetSafeNormal();
	float dot = FVector::DotProduct(vector, normal);
	return vector-(dot*normal);
}
