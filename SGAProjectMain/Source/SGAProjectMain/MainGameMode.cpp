// Fill out your copyright notice in the Description page of Project Settings.


#include "MainGameMode.h"

#include "Kismet/GameplayStatics.h"
#include "CGameInstance.h"
#include "Character/PlayerCharacter.h"
#include "Character/HellDiver/HellDiverInvenComponent.h"
#include "Object/Map/EscapePlane.h"
#include "Game/EnemyReinforceManager.h"
void AMainGameMode::BeginPlay()
{
    Super::BeginPlay();

    if(_enemyReinforceManagerClass)
        _enemyReinforceManager = GetWorld()->SpawnActor<AEnemyReinforceManager>(_enemyReinforceManagerClass, FVector::ZeroVector, FRotator::ZeroRotator);


   

}

void AMainGameMode::StartPlay()
{
    Super::StartPlay();
}

void AMainGameMode::CallEscapePlane()
{
    UWorld* world = GetWorld();
    if (!world) return;

    AEscapePlane* escapePlane = world->SpawnActor<AEscapePlane>(_escapePlaneClass, _planeSpawnLoc, FRotator::ZeroRotator);
}

void AMainGameMode::OnBattleEnd() // 게임이 끝났을 경우
{
    UCGameInstance* GI = Cast<UCGameInstance>(GetGameInstance());
    if (!GI) return;

    APlayerCharacter* player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (player)
    {
        FSampleBundle earnedSample = player->GetInvenComponent()->GetSampleBundle();
        GI->AddEarnedSample(earnedSample); // 들고 있는 샘플 합산해서 저장
    }

    UGameplayStatics::OpenLevel(this, FName("Lobby")); // 레벨 이동

    UE_LOG(LogTemp, Log, TEXT("Move Level!"))
}
