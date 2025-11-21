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
	FName GetOperationID() { return _operationID; }
	FText GetOperationName() { return _operationName; }
	FText GetOperationDesc() { return _operationDesc; }
	TArray<class UMissionDataAsset*> GetMissions() { return _missions; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Operation")
	FName _operationID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Operation")
	FText _operationName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Operation")
	FText _operationDesc;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Operation")
	TArray<class UMissionDataAsset*> _missions;
};
