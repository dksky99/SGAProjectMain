// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PreDeploymentState.generated.h"

/**
 * 
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMissionSelected, bool);

UENUM()
enum class EMissionState : uint8
{
	None,
	Available,
	Cleared,
	Failed
};

UCLASS()
class SGAPROJECTMAIN_API UPreDeploymentState : public UObject
{
	GENERATED_BODY()

public:
	UPreDeploymentState();
	void ApplySaveGameData(class UCSaveGame* saveGame);

	// 총
	void SetGunID(int32 id);

	void SetPrimaryGunID(int32 id) { _primaryGunID = id; }
	int32 GetPrimaryGunID() { return _primaryGunID; }

	void SetSecondaryGunID(int32 id) { _secondaryGunID = id; }
	int32 GetSecondaryGunID() { return _secondaryGunID; }

	void SetStratagemID(int32 index, int32 id);
	TArray<int32> GetStratagemIDs() { return _stratagemIDs; }

	// 임무 및 미션 
	void SetCurOperation(class UOperationDataAsset* op);
	class UOperationDataAsset* GetCurOperation() const { return _curOperation; }

	void SetCurMission(class UMissionDataAsset* mission);
	class UMissionDataAsset* GetCurMission() const { return _curMission; }

	FOnMissionSelected _missionSelectedEvent;

	void ApplyMissionResult(class UMissionDataAsset* mission, bool isCleared);
	void ResetOperation();

	bool IsOperationCleared();
	bool IsOperationFailed();
	bool IsMissionCleared(UMissionDataAsset* mission);

	int32 GetClearedMissionsNum();
	TMap<UMissionDataAsset*, EMissionState> GetMissionStates() { return _missions; }

private:
	int32 _primaryGunID = 1;
	int32 _secondaryGunID = 101;

	UPROPERTY()
	TArray<int32> _stratagemIDs;

	UPROPERTY()
	class UOperationDataAsset* _curOperation;
	UPROPERTY()
	class UMissionDataAsset* _curMission;
	UPROPERTY()
	TMap<UMissionDataAsset*, EMissionState> _missions;
};