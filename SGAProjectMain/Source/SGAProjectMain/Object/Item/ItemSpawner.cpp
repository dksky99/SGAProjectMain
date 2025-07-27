// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemSpawner.h"
#include "NavigationSystem.h"

// Sets default values
AItemSpawner::AItemSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AItemSpawner::BeginPlay()
{
	Super::BeginPlay();
	SpawnItem();
}

// Called every frame
void AItemSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItemSpawner::SpawnItem()
{
    for (const auto& item : _spawnList)
    {
        for (int32 i = 0; i < item._count; i++)
        {
            FVector spawnLoc = FindSpawnLocation();
            GetWorld()->SpawnActor<AActor>(item._spawnClass, spawnLoc, FRotator::ZeroRotator);
        }
    }
}

FVector AItemSpawner::FindSpawnLocation()
{
    UNavigationSystemV1* navSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!navSys) 
        return GetActorLocation();

    const int maxAttempts = 100;

    for (int i = 0; i < maxAttempts; ++i)
    {
        FNavLocation navLoc;
        if (!navSys->GetRandomPointInNavigableRadius(GetActorLocation(), 1500.f, navLoc)) // 액터 위치를 기준으로 1500.f 안의 범위에서, NavMesh에 속한 랜덤 위치 획득
            continue; // 실패했다면 재시도

        FVector start = navLoc.Location + FVector(0, 0, 500.f); // 찾은 위치보다 500 위에서 시작
        FVector end = navLoc.Location - FVector(0, 0, 2000.f); // 찾은 위치보다 2000 아래로 라인트레이스
        FHitResult hit;

        if (GetWorld()->LineTraceSingleByChannel(hit, start, end, ECC_Visibility))
        {
            return hit.Location + FVector(0, 0, 10.f);
        }
    }

    // 전부 실패 시 자기 위치 반환
    UE_LOG(LogTemp, Warning, TEXT("Can't find valid spawn location"));
    return GetActorLocation();
}

