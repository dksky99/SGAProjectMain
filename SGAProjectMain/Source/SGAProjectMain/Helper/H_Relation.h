// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class SGAPROJECTMAIN_API H_Relation
{
public:
	static bool IsInSight(class AActor* user, class AActor* target, float maxAngle);
	static bool IsInSightYaw(class AActor* user, class AActor* target, float maxAngle);
	static bool IsInDistance(class AActor* user, class AActor* target, float maxDistance, float minDistance);
	static FVector LocOfDistanceFromTarget(class AActor* user, class AActor* target, float distance );
	static FVector FindFloorFromLoc(class AActor* user, float maxDistance,bool useAnotherLoc,  FVector loc = FVector::ZeroVector);

};
