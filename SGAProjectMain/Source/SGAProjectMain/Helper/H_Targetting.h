// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


UENUM(BlueprintType)
enum class ETargettingType :uint8
{
	MostNear  UMETA(DisplayName = "MostNear"),
	MostFar  UMETA(DisplayName = "MostFar"),
	MostLowHP UMETA(DisplayName = "MostLowHP"),
	MostHighHP UMETA(DisplayName = "MostHighHP"),
	Revenge UMETA(DisplayName = "Revenge"),
	Max
};
/**
 * 
 */
class SGAPROJECTMAIN_API H_Targetting
{
public:
	static AActor* Targetting(ETargettingType targettype, ETargettingType defaultTargetType, TArray<AActor*>& arr, const AActor* existing, AActor* curPawn);

	static AActor* MostNearByTarget(TArray<AActor*>& arr, const AActor* existing,  AActor* curPawn);
	static AActor* MostFarByTarget(TArray<AActor*>& arr, const AActor* existing,  AActor* curPawn);
	//static AActor* MostLowHpTarget(TArray<AActor*>& arr, const AActor* existing, AActor* curPawn);
	//static AActor* MostHighHpTarget(TArray<AActor*>& arr, const AActor* existing, AActor* curPawn);
	//static AActor* RevengeTarget(TArray<AActor*>& arr, const AActor* existing,  AActor* curPawn) ;
	
};
