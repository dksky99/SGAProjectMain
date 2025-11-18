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

	// 총 관련 초기화
	void SetGunID(int32 id);

	void SetPrimaryGunID(int32 id) { _primaryGunID = id; }
	int32 GetPrimaryGunID() { return _primaryGunID; }

	void SetSecondaryGunID(int32 id) { _secondaryGunID = id; }
	int32 GetSecondaryGunID() { return _secondaryGunID; }

	void SetStratagemID(int32 index, int32 id);
	TArray<int32> GetStratagemIDs() { return _stratagemIDs; }

	// 작전 및 미션 관련 초기화 
	void SetCurOperation(class UOperationDataAsset* op);
	class UOperationDataAsset* GetCurOperation() const { return _curOperation; }

	void SetCurMission(class UMissionDataAsset* mission);
	class UMissionDataAsset* GetCurMission() const { return _curMission; }

	FOnMissionSelected _missionSelectedEvent;

	void ApplyMissionResult(bool isCleared);
	void ClearOperation();

	bool IsOperationCleared();
	bool IsOperationFailed();

	int32 GetClearedMissionsNum();

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