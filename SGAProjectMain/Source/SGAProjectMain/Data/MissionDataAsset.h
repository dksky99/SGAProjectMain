// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MissionDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class SGAPROJECTMAIN_API UMissionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	bool IsOptionalObjectiveIDValid(FName objectiveID);
	FName GetMainObjectiveID();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Planet Operation")
	FName _missionName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Planet Operation")
	FText _missionDesc;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Planet Operation")
	class UObjectiveDataAsset* _mainObjective;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Planet Operation")
	TArray<class UObjectiveDataAsset*> _optionalObjectives;
};
