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
#include "Game/PreDeployment/PreDeploymentState.h"
#include "Data/MissionDataAsset.h"

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

    _planeBeacon = Cast<ADropPlaneBeacon>(UGameplayStatics::GetActorOfClass(this, ADropPlaneBeacon::StaticClass()));
}

void AMainGameMode::StartPlay()
{
    Super::StartPlay();

    GetWorldTimerManager().SetTimer(_missionTimerHandle, this, &AMainGameMode::UpdateTimer, 1.0f, true);
}

void AMainGameMode::OnObjectiveCleared(FName objectiveID)
{
    // 메인 목표 클리어 시 탈출 가능
    if (_missionProgress._curMission->GetMainObjectiveID() == objectiveID)
    {
        _missionProgress._isMainObjectiveCleared = true;
        EnableExtraction();
        return;
	}
    else
    {
        // 추가 목표 클리어 시 기록
        if (_missionProgress._curMission->IsOptionalObjectiveIDValid(objectiveID))
        {
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

    APlayerCharacter* player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (player)
    {
		player->AddMissionSlot(_planeMissionIcon, FString("Extraction Avaliable"));
    }
}

void AMainGameMode::CallEscapePlane()
{
	if (!_escapePlaneClass) return;

    UWorld* world = GetWorld();
    if (!world) return;

    FRotator rotation(0.f, 90.f, 0.f);

    AEscapePlane* escapePlane = world->SpawnActor<AEscapePlane>(_escapePlaneClass, _planeSpawnLoc, rotation);
	escapePlane->_helldiverExtractEvent.AddLambda([this]()
        {
			_missionProgress._extractedHelldiversNum += 1;
        });
    _escapePlane = escapePlane;
}

void AMainGameMode::EndBattle() // 게임이 끝났을 경우
{
    UCGameInstance* GI = Cast<UCGameInstance>(GetGameInstance());
    if (!GI) return;

    GI->GetPreDeployState()->ApplyMissionResult(_missionProgress._isMainObjectiveCleared);

	FPlayerCurrency reward = CalculateMissionReward();
	GI->AddRewardCurrency(reward);

    UGameplayStatics::OpenLevel(this, FName("Lobby")); // 레벨 이동

    UE_LOG(LogTemp, Log, TEXT("Move Level!"))
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
}

FPlayerCurrency AMainGameMode::CalculateMissionReward()
{
    FPlayerCurrency reward;

	// 메인 목표 클리어 보상
    if (_missionProgress._isMainObjectiveCleared)
    {
        reward.Add(ECurrencyType::Experience, 100);
        reward.Add(ECurrencyType::RequisitionSlips, 500);
	}

	// 추가 목표 클리어 보상
	reward.Add(ECurrencyType::Experience, 50 * _missionProgress._completedOptionalObjectives.Num());
	reward.Add(ECurrencyType::RequisitionSlips, 200 * _missionProgress._completedOptionalObjectives.Num());

	// 헬다이버 추출 보상
	reward.Add(ECurrencyType::Experience, 20 * _missionProgress._extractedHelldiversNum);
	reward.Add(ECurrencyType::RequisitionSlips, 50 * _missionProgress._extractedHelldiversNum);

    // 탈출한 헬다이버가 있다면 샘플 획득
    if (_missionProgress._extractedHelldiversNum > 0)
    {
        APlayerCharacter* player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
        if (player)
        {
            FSampleBundle earnedSample = player->GetInvenComponent()->GetSampleBundle();
			reward._samples = earnedSample;
        }
    }

    return FPlayerCurrency();
}
