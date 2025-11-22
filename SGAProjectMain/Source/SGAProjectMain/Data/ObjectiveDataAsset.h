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
	FText GetObjectiveName() { return _objectiveName; }
	FText GetObjectiveDesc() { return _objectiveDesc; }
	class UTexture2D* GetObjectiveIcon() { return _objectiveIcon; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Objective")
	FName _objectiveID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Objective")
	FText _objectiveName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Objective")
	FText _objectiveDesc;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Objective")
	class UTexture2D* _objectiveIcon;
};
