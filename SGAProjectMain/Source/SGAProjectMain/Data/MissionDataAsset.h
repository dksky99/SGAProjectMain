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
	float GetTimeLimitSeconds() { return _timeLimitSeconds; }
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Mission")
	FName _missionName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Mission")
	FText _missionDesc;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Mission")
	class UObjectiveDataAsset* _mainObjective;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Mission")
	TArray<class UObjectiveDataAsset*> _optionalObjectives;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Mission")
	float _timeLimitSeconds;
};
