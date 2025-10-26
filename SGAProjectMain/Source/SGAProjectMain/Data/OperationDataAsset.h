// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OperationDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class SGAPROJECTMAIN_API UOperationDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	TArray<class UMissionDataAsset*> GetMissions() { return _missions; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Planet Operation")
	FName _operationName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Planet Operation")
	FText _operationDesc;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Planet Operation")
	TArray<class UMissionDataAsset*> _missions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Planet Operation")
	TArray<int32> _reward;
};
