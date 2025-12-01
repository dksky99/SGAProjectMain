// Fill out your copyright notice in the Description page of Project Settings.


#include "ReinforceSquadSpawner.h"

void AReinforceSquadSpawner::ActivateSpawner(AEnemySquad* squad, FVector loc)
{

	if (squad == nullptr)
		return;
	SetActorLocation(loc);

	UE_LOG(LogTemp, Display, TEXT("Spawner MoveTo %f %f %f "),loc.X,loc.Y,loc.Z);
	Super::ActivateSpawner(squad, loc);

	// 증원 스쿼드는 활성화와 동시에 소환을 시작한다. 3초뒤인 이유는 네비 영역을 활성화하는데 시간여유가 필요해서.
	GetWorld()->GetTimerManager().SetTimer(_spawnTimer, this, &ASquadSpawner::SpawnUnits, 3.0f, false);
}

void AReinforceSquadSpawner::CallFinishAction()
{
	// 증원 스쿼드는 소환이 끝나면 스포너를 돌려보낸다.
	DeactivateSpawner();
}
