// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CGameInstance.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FSelectedStratagemSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<class AStratagem>> SelectedStratagems;
};

UCLASS()
class SGAPROJECTMAIN_API UCGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite)
	TMap<FName, FSelectedStratagemSet> AllPlayerStratagemSets;
};
