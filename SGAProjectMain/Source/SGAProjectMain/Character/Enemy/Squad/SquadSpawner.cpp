// Fill out your copyright notice in the Description page of Project Settings.


#include "SquadSpawner.h"
#include "../EnemySquad.h"
#include "../Enemy.h"

#include "NavigationSystem.h"
#include "NavigationInvokerComponent.h"
// Sets default values
ASquadSpawner::ASquadSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	

	_navInvokerComponent = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("NavInvoker"));

}

void ASquadSpawner::ActivateSpawner(AEnemySquad* squad,FVector loc)
{
	if (squad == nullptr)
		return;
	_squad = squad;

	//스포너가 활성화되면 주변에 네비 영역이 설치되게 한다.
	_navInvokerComponent->SetActive(true, true);

}

void ASquadSpawner::DeactivateSpawner()
{
	_squad = nullptr;
	_navInvokerComponent->SetActive(false, true);

}

void ASquadSpawner::CallRemainUnit()
{
	//만약 어떤이유로든 비활성화되어 스포너로부터 스쿼드가끊기면 멈춤.
	if (_squad == nullptr)
		return;
	AEnemy* unit =_squad->CheckExtraUnit();

	//남은 유닛이 없다.혹은 정해진 횟수 반복. 증원스쿼드라면 스쿼드의 전부를 쏟아낼것이고 주둔스쿼드라면 일정유닛만 소환할것.
	if (unit == nullptr||_curSpawnCount==0)
		return;
	//위치 설정.
	//컨트롤러 빙의
	if (unit->CombineController())
	{
		unit->SetActorLocation(GetSpawnPoint(), false, nullptr, ETeleportType::TeleportPhysics);
		unit->Spawn();

	}
	


	//한 유닛을 소환하고 다음유닛을 소환할떄 텀이 있다.
	float interval = (float)(FMath::RandRange(_minSpawnInterval, _maxSpawnInterval));
	GetWorld()->GetTimerManager().SetTimer(_spawnTimer, this, &ASquadSpawner::CallRemainUnit, interval, false);
}

void ASquadSpawner::SpawnUnits()
{
	CallRemainUnit();
	
}

FVector ASquadSpawner::GetSpawnPoint()
{

	FVector pos = GetActorLocation();
	//NavMesh 찾기
	auto naviSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());

	//반환받을 랜덤한 위치.
	FNavLocation randLocation;
	//일정 반경안의 랜덤한 지점을 가져오는 함수
	if (naviSystem->GetRandomPointInNavigableRadius(pos, _callRadius, randLocation))
	{
		pos = randLocation;
	}

	return pos;
}

void ASquadSpawner::SpawnerCoolDownFinish()
{
	_spawnerReady = true;
}

// Called when the game starts or when spawned
void ASquadSpawner::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASquadSpawner::CallFinishAction()
{
}


