// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Object/Item/SampleResources.h"
#include "CGameInstance.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	None,
	Lobby,
	InMission
};

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


	UCGameInstance();
	virtual void Init() override;

	UPROPERTY(BlueprintReadWrite)
	TMap<FName, FSelectedStratagemSet> AllPlayerStratagemSets;

	struct FGunData GetGunDataFromTable(int32 id);
	TSubclassOf<class AGunBase> GetGunClassFromTable(int32 id);
	class UTexture2D* GetGunPreviewFromTable(int32 id);
	class UDataTable* GetGunTable() { return _gunTable; }

	struct FStratagemSlot GetStratagemSlotFromTable(int32 id);
	TSubclassOf<class AStratagem> GetStratagemClassFromTable(int32 id);
	class UDataTable* GetStratagemTable() { return _stratagemTable; }

	void AddEarnedSample(const FSampleBundle& earnedSample);
	FSampleBundle GetSavedSample() { return _savedSample; }


	class UPreDeploymentState* GetPreDeployState() { return _preDeployState; }

	void SetGamePhase(EGamePhase newPhase) { _curGamePhase = newPhase; }
	EGamePhase GetGamePhase() const { return _curGamePhase; }



private:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UDataTable* _gunTable;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UDataTable* _stratagemTable;

	UPROPERTY(EditAnywhere, Category = "Sample")
	FSampleBundle _savedSample;

	UPROPERTY()
	UPreDeploymentState* _preDeployState;

	EGamePhase _curGamePhase = EGamePhase::None;
};
