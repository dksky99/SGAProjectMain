// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class SGAPROJECTMAIN_API H_Rotator
{
public:
	static FVector ProjectVectorOnPlane(const FVector& vector, const FVector& planeNormal);
};
