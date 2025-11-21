// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Data/PlayerCurrency.h"
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

	// 무기 데이터
	struct FGunData GetGunDataFromTable(int32 id);
	TSubclassOf<class AGunBase> GetGunClassFromTable(int32 id);
	class UTexture2D* GetGunPreviewFromTable(int32 id);
	class UDataTable* GetGunTable() { return _gunTable; }

	struct FStratagemSlot GetStratagemSlotFromTable(int32 id);
	TSubclassOf<class AStratagem> GetStratagemClassFromTable(int32 id);
	class UDataTable* GetStratagemTable() { return _stratagemTable; }

	// 재화
	void AddRewardCurrency(const FPlayerCurrency& reward);
	//void AddCurrency(ECurrencyType type, int32 amount = 1);
	//void AddEarnedSample(const FSampleBundle& earnedSample);
	FSampleBundle GetSavedSample();

	class UPreDeploymentState* GetPreDeployState() { return _preDeployState; }

	void SetGamePhase(EGamePhase newPhase) { _curGamePhase = newPhase; }
	EGamePhase GetGamePhase() const { return _curGamePhase; }


private:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UDataTable* _gunTable;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UDataTable* _stratagemTable;

	// 작전 및 미션 데이터
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TArray<class UOperationDataAsset*> _allOperations;
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TArray<class UMissionDataAsset*> _allMissions;
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TArray<class UObjectiveDataAsset*> _allObjectives;

	UPROPERTY()
	TMap<FName, UOperationDataAsset*> _operationMap;
	UPROPERTY()
	TMap<FName, UMissionDataAsset*> _missionMap;
	UPROPERTY()
	TMap<FName, UObjectiveDataAsset*> _objectiveMap;


	UPROPERTY()
	UPreDeploymentState* _preDeployState;

	EGamePhase _curGamePhase = EGamePhase::None;

	// 재화
	UPROPERTY(VisibleAnywhere, Category = "Game/Currency")
	FPlayerCurrency _playerCurrency;
};
