// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "MainGameMode.generated.h"

/**
 * 
 */
USTRUCT()
struct FMissionProgress
{
	GENERATED_BODY()

	UPROPERTY()
	class UMissionDataAsset* _curMission = nullptr;

	bool _isMainObjectiveCleared = false;

	int _extractedHelldiversNum = 0;

	UPROPERTY()
	TSet<FName> _completedOptionalObjectives;
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
private:
	void UpdateTimer(); 
	struct FPlayerCurrency CalculateMissionReward();

	UPROPERTY()
	FMissionProgress _missionProgress;

	UPROPERTY(EditAnywhere, Category = "Game/Plane")
	TSubclassOf<class AEscapePlane> _escapePlaneClass;

	UPROPERTY()
	class AEscapePlane* _escapePlane;

	UPROPERTY(EditAnywhere, Category = "Game/Plane")
	FVector _planeSpawnLoc;

	UPROPERTY(EditAnywhere, Category = "Game/Console")
	class ADropPlaneBeacon* _planeBeacon;

	UPROPERTY(EditAnywhere, Category = "Game/UI")
	class UTexture2D* _planeMissionIcon; // 임시. 추후 삭제 예정

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
