// Fill out your copyright notice in the Description page of Project Settings.


#include "MainGameMode.h"

#include "Kismet/GameplayStatics.h"
#include "CGameInstance.h"
#include "Character/PlayerCharacter.h"
#include "Character/HellDiver/HellDiverInvenComponent.h"
#include "Character/HellDiver/HelldiverReinforceManager.h"
#include "Object/Map/EscapePlane.h"
#include "Object/Map/DropPlaneBeacon.h"
#include "Object/Map/TerminalConsole.h"
#include "Game/EnemyReinforceManager.h"
#include "Game/EnemyPatrolManager.h"
#include "Game/EnemyGarrisonManager.h"
#include "Game/PreDeployment/PreDeploymentState.h"
#include "Data/OperationDataAsset.h"
#include "Data/MissionDataAsset.h"
#include "UI/MissionResultWidget.h"

void AMainGameMode::BeginPlay()
{
    Super::BeginPlay();

    UCGameInstance* GI = Cast<UCGameInstance>(GetGameInstance());
    if (!GI) return;

    GI->SetGamePhase(EGamePhase::InMission);

    auto mission = GI->GetPreDeployState()->GetCurMission();
    if (mission)
    {
        _missionProgress._curMission = mission;
        _remainingTime = mission->GetTimeLimitSeconds();
    }

    if(_enemyReinforceManagerClass)
        _enemyReinforceManager = GetWorld()->SpawnActor<AEnemyReinforceManager>(_enemyReinforceManagerClass, FVector::ZeroVector, FRotator::ZeroRotator);

   
    if (_helldiverReinforceManagerClass)
        _helldiverReinforceManager = GetWorld()->SpawnActor<AHelldiverReinforceManager>(_helldiverReinforceManagerClass, FVector::ZeroVector, FRotator::ZeroRotator);
    
    
    //맵의 매니저들 찾기.
    UWorld* World = GetWorld();
    if (!World) return;
    TArray<AActor*> FoundActors;


    UGameplayStatics::GetAllActorsOfClass(World, AEnemyPatrolManager::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        _enemyPatrolManager = Cast<AEnemyPatrolManager>(FoundActors[0]);
    }

    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AEnemyGarrisonManager::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        _enemyGarrisonManager = Cast<AEnemyGarrisonManager>(FoundActors[0]);
    }
}

void AMainGameMode::StartPlay()
{
    Super::StartPlay();

    _planeBeacon = Cast<ADropPlaneBeacon>(UGameplayStatics::GetActorOfClass(this, ADropPlaneBeacon::StaticClass()));

    GetWorldTimerManager().SetTimer(_missionTimerHandle, this, &AMainGameMode::UpdateTimer, 1.0f, true);
}

void AMainGameMode::OnObjectiveCleared(FName objectiveID)
{
    // 메인 목표 클리어 시 탈출 가능
	if (!_missionProgress._curMission) return;

    if (_missionProgress._curMission->GetMissionID() == objectiveID)
    {
		_objectiveCompletedEvent.Broadcast(objectiveID);
		_missionCompletedEvent.Broadcast();
        _missionProgress._isMissionCleared = true;
        EnableExtraction();
        return;
	}
    else
    {
        // 추가 목표 클리어 시 기록
        if (_missionProgress._curMission->IsOptionalObjectiveIDValid(objectiveID))
        {
            _objectiveCompletedEvent.Broadcast(objectiveID);
            _missionProgress._completedOptionalObjectives.Add(objectiveID);
		}
    }
}

void AMainGameMode::EnableExtraction()
{
    if (_planeBeacon)
    {
        _planeBeacon->SetInteractable(true); // 꺼져있던 비콘 활성화
	}
}

void AMainGameMode::CallEscapePlane()
{
	if (!_escapePlaneClass) return;

    UWorld* world = GetWorld();
    if (!world) return;

    FRotator rotation(0.f, 90.f, 0.f);

    AEscapePlane* escapePlane = world->SpawnActor<AEscapePlane>(_escapePlaneClass, _planeSpawnLoc, rotation);
	escapePlane->_helldiverExtractEvent.AddUObject(this, &AMainGameMode::OnHelldiverExtracted);
    _escapePlane = escapePlane;
}

void AMainGameMode::EndBattle() // 게임이 끝났을 경우
{
	GetWorldTimerManager().ClearTimer(_missionTimerHandle);
   
    if (!_missionProgress._curMission) return;
	float timeLimit = _missionProgress._curMission->GetTimeLimitSeconds();
    _missionResult._remainingTimeRatio = FMath::Clamp(_remainingTime / timeLimit, 0.f, 1.f);

    UCGameInstance* GI = Cast<UCGameInstance>(GetGameInstance());
    if (!GI) return;

	auto preDeployState = GI->GetPreDeployState();

	_missionResult._operation = preDeployState->GetCurOperation();
    _missionResult._mission = _missionProgress._curMission;
    _missionResult._completedOptionalObjectives = _missionProgress._completedOptionalObjectives;
    _missionResult._extractedHelldiversNum = _missionProgress._extractedHelldiversNum;
    _missionResult._isMissionCleared = _missionProgress._isMissionCleared;
    _missionResult._clearedMissionNum = preDeployState->GetClearedMissionsNum();
    if (_missionProgress._isMissionCleared)
        _missionResult._clearedMissionNum++;

	CalculateMissionReward();
    GI->ApplyMissionResult(_missionResult);
	//GI->AddRewardCurrency(_missionResult._totalReward);

    if (_resultWidgetClass)
    {
        auto resultWidget = CreateWidget<UMissionResultWidget>(GetWorld(), _resultWidgetClass);

        if (resultWidget)
        {
            resultWidget->AddToViewport();
            resultWidget->InitializeWidget(_missionResult);
			resultWidget->_rewardFlowFinishedEvent.BindUObject(this, &AMainGameMode::OpenLobby);
        }
    }
}

void AMainGameMode::UpdateTimer()
{
    if (_remainingTime <= 0.f)
    {
        GetWorldTimerManager().ClearTimer(_missionTimerHandle);

        if (_escapePlane) _escapePlane->StartTimerToTakeOff();
        else CallEscapePlane();
        return;
    }

	_remainingTime -= 1.f;

	_timerUpdatedEvent.Broadcast(_remainingTime);
}

void AMainGameMode::CalculateMissionReward()
{
	// 메인 목표 클리어 보상
    FMissionReward mainReward;
    mainReward._category = ERewardCategory::MainObjective;
    if (_missionProgress._isMissionCleared)
    {   
		mainReward._experience = 100;
        mainReward._requisitionSlips = 500;

        // 메달은 메인 목표 성공 시에만 지급
        if (auto op = _missionResult._operation)
        {
            if (op->GetRewardMedals().IsValidIndex(_missionResult._clearedMissionNum - 1))
            {
                int32 medal = op->GetRewardMedals()[_missionResult._clearedMissionNum - 1];
                _missionResult._totalReward.Add(ECurrencyType::Medals, medal);
			}
        }
	}
    _missionResult._missionRewards.Add(mainReward);

	// 추가 목표 클리어 보상
	FMissionReward optionalReward;
	optionalReward._category = ERewardCategory::OptionalObjectives;
    optionalReward._experience = 50 * _missionProgress._completedOptionalObjectives.Num();
    optionalReward._requisitionSlips = 200 * _missionProgress._completedOptionalObjectives.Num();
    _missionResult._missionRewards.Add(optionalReward);

	// 헬다이버 추출 보상
    FMissionReward extractionReward;
	extractionReward._category = ERewardCategory::HelldiversExtracted;
    extractionReward._experience = 20 * _missionProgress._extractedHelldiversNum;
    extractionReward._requisitionSlips = 50 * _missionProgress._extractedHelldiversNum;
    _missionResult._missionRewards.Add(extractionReward);

    // 남은 시간 보상
    FMissionReward remainingTimeReward;
	remainingTimeReward._category = ERewardCategory::MissionTimeRemaining;
    remainingTimeReward._experience = 100 * _missionResult._remainingTimeRatio;
    remainingTimeReward._requisitionSlips = 400 * _missionResult._remainingTimeRatio;
    _missionResult._missionRewards.Add(remainingTimeReward);

    // 탈출한 헬다이버가 있다면 샘플 획득
    if (_missionProgress._extractedHelldiversNum > 0)
    {
        APlayerCharacter* player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
        if (player)
        {
            FSampleBundle earnedSample = player->GetInvenComponent()->GetSampleBundle();
            _missionResult._totalReward.AddSample(earnedSample);
        }
    }

    for (const FMissionReward& reward : _missionResult._missionRewards)
    {
        if (reward._experience != 0)
			_missionResult._totalExperience += reward._experience;
        if (reward._requisitionSlips != 0)
            _missionResult._totalReward.Add(ECurrencyType::RequisitionSlips, reward._requisitionSlips);
    }
}

void AMainGameMode::OnHelldiverExtracted()
{
    _missionProgress._extractedHelldiversNum += 1;
}

void AMainGameMode::OpenLobby()
{
	UGameplayStatics::OpenLevel(this, FName("Lobby"));
}
