// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ObjectiveDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class SGAPROJECTMAIN_API UObjectiveDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	FName GetObjectiveID() { return _objectiveID; }

protected:
	UPROPERTY(EditAnywhere)
	FName _objectiveID;
	
};
