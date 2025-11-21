// Fill out your copyright notice in the Description page of Project Settings.


#include "FactorySquadSpawner.h"
#include "Kismet/GameplayStatics.h"
AFactorySquadSpawner::AFactorySquadSpawner()
{

    PrimaryActorTick.bCanEverTick = true;

    PrimaryActorTick.TickInterval = 0.5f;
}
void AFactorySquadSpawner::Tick(float DeltaTime)
{

	Super::Tick(DeltaTime);

    CheckDistanceToTarget();

}

void AFactorySquadSpawner::CheckDistanceToTarget()
{
    // 인덱스 0은 항상 첫 번째 로컬 플레이어입니다.
    APlayerController* PlayerController0 = UGameplayStatics::GetPlayerController(
        GetWorld(),
        0 // 플레이어 인덱스 (0, 1, 2, ...)
    );

    if (PlayerController0)
    {
        auto pawn = PlayerController0->GetPawn();
        if (pawn == nullptr)
            return;
        float distance = GetDistanceTo(pawn);
        if (distance < _alertRange)
            SpawnUnits();
    }
}
