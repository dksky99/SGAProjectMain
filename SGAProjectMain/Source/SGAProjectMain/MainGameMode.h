// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Data/MissionResult.h"
#include "MainGameMode.generated.h"

/**
 * 
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnObjectiveCompleted, FName)
DECLARE_MULTICAST_DELEGATE(FOnMissionCompleted)

USTRUCT()
struct FMissionProgress
{
	GENERATED_BODY()

	UPROPERTY()
	class UMissionDataAsset* _curMission = nullptr;

	UPROPERTY()
	TSet<FName> _completedOptionalObjectives;
	
	bool _isMissionCleared = false; // 메인 목표 클리어 여부
	int32 _extractedHelldiversNum = 0; // 탈출한 헬다이버 수
};

UCLASS()
class SGAPROJECTMAIN_API AMainGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void StartPlay() override;

	void OnObjectiveCleared(FName objectiveID);
	void EnableExtraction();
	void CallEscapePlane();
	void EndBattle();

	class AEnemyReinforceManager* GetEnemyReinforceManager() { return _enemyReinforceManager; }
	class AHelldiverReinforceManager* GetHelldiverReinforceManager() { return _helldiverReinforceManager; }

	bool IsTimeOver() const { return _remainingTime <= 0.f; }

	FOnObjectiveCompleted _objectiveCompletedEvent;
	FOnMissionCompleted _missionCompletedEvent;

protected:
	void UpdateTimer(); 
	void CalculateMissionReward();

	void OnHelldiverExtracted();
	void OpenLobby();

	UPROPERTY()
	FMissionProgress _missionProgress;
	UPROPERTY()
	FMissionResult _missionResult;

	UPROPERTY(EditAnywhere, Category = "Game/UI")
	TSubclassOf<class UUserWidget> _resultWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Game/Plane")
	TSubclassOf<class AEscapePlane> _escapePlaneClass;

	UPROPERTY()
	class AEscapePlane* _escapePlane;

	UPROPERTY(EditAnywhere, Category = "Game/Plane")
	FVector _planeSpawnLoc;

	UPROPERTY(EditAnywhere, Category = "Game/Console")
	class ADropPlaneBeacon* _planeBeacon;

	UPROPERTY(EditAnywhere, Category = "Game/GamePlay")
	TSubclassOf<class AEnemyReinforceManager> _enemyReinforceManagerClass;

	UPROPERTY(EditAnywhere, Category = "Game/GamePlay")
	TSubclassOf<class AHelldiverReinforceManager> _helldiverReinforceManagerClass;

	UPROPERTY(VisibleAnywhere, Category = "Game/EnemyReinforce")
	class AEnemyReinforceManager* _enemyReinforceManager;
	UPROPERTY(VisibleAnywhere, Category = "Game/EnemyReinforce")
	class AHelldiverReinforceManager* _helldiverReinforceManager;

	float _remainingTime = 0.f;
	FTimerHandle _missionTimerHandle;
};
