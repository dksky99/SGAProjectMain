// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Object/Item/SampleResources.h"
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
	virtual void Init() override;

	UPROPERTY(BlueprintReadWrite)
	TMap<FName, FSelectedStratagemSet> AllPlayerStratagemSets;

	struct FGunData GetGunDataFromTable(int32 id);
	TSubclassOf<class AGunBase> GetGunClassFromTable(int32 id);

	struct FStratagemSlot GetStratagemSlotFromTable(int32 id);
	TSubclassOf<class AStratagem> GetStratagemClassFromTable(int32 id);

	void AddEarnedSample(const FSampleBundle& earnedSample);
	FSampleBundle GetSavedSample() { return _savedSample; }

	class UPreDeploymentState* GetPreDeployState() { return _preDeployState; }

private:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UDataTable* _gunTable;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UDataTable* _stratagemTable;

	UPROPERTY(EditAnywhere, Category = "Sample")
	FSampleBundle _savedSample;

	UPROPERTY()
	UPreDeploymentState* _preDeployState;
};
