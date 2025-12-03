// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyGarrisonManager.h"
#include "../Character/Enemy/Squad/FactorySquadSpawner.h"
#include "../Character/Enemy/EnemySquad.h"
#include "../MainGameMode.h"


// Sets default values
AEnemyGarrisonManager::AEnemyGarrisonManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemyGarrisonManager::BeginPlay()
{
	Super::BeginPlay();
	InitGarrison();
}

void AEnemyGarrisonManager::InitGarrison()
{
	for (auto data : _garrisonDatas)
	{
		//주둔 명령
		data.Squad->Command_Stationed(data.RellyPoint->GetActorLocation());
		
		for (auto spawner : data.Spawners)
		{
			spawner->ActivateSpawner(data.Squad);
			spawner->_onFactoryDestroyed.AddDynamic(this, &AEnemyGarrisonManager::CheckFactoryDestroyed);
		}
		
	}
}

void AEnemyGarrisonManager::CheckFactoryDestroyed(const AEnemySquad* squad)
{
	FGarrisonData* foundData = _garrisonDatas.FindByPredicate([squad](const FGarrisonData& Data)
		{
			// FGarrisonData 구조체의 Squad 필드가 TargetSquad와 일치하는지 확인
			return Data.Squad == squad;
		});
	foundData->AddDestroyedCount();

	if (foundData->Spawners.Num() == foundData->GetDestroyedCount())
	{
		CheckMissionComplete(foundData);
		//전부 파괴되었다 미션이 있었다면 보상을 받는다. 또한 여기있던 스쿼드를 증원스쿼드에 넘겨버리면 남는 스쿼드를 쓸 수 있을듯하다.
	}

}

void AEnemyGarrisonManager::CheckMissionComplete(const FGarrisonData* const data)
{
	AMainGameMode* GM = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this));
	if (GM)
	{
		GM->OnObjectiveCleared(data->_objectiveID);
	}
}

// Called every frame
void AEnemyGarrisonManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
