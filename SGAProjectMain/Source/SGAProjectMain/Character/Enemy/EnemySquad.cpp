// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySquad.h"
#include "Enemy.h"
#include "PatrolComponent.h"
#include "../../Controller/EnemyController.h"

// Sets default values
AEnemySquad::AEnemySquad()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	_rootComponent = CreateDefaultSubobject<USceneComponent>("RootComp");
	RootComponent = _rootComponent;
	
	_spawnPoint = CreateDefaultSubobject<USceneComponent>("SpawnPoint");
	_spawnPoint->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AEnemySquad::BeginPlay()
{
	Super::BeginPlay();


	Init();
}

// Called every frame
void AEnemySquad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemySquad::Init()
{
	
	FActorSpawnParameters param;
	param.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	for (const TPair<TSubclassOf<AEnemy>, int>& unitPair : _unitList)
	{
		TSubclassOf<AEnemy> enemyClass = unitPair.Key;
		int32 count = unitPair.Value;
		
		FEnemyUnit& enemyArray = _unitPool.FindOrAdd(enemyClass);

					UE_LOG(LogTemp, Display, TEXT("SpawnCount %d"),count);
		for (int i = 0; i < count; ++i)
		{
			AEnemy* enemy = GetWorld()->SpawnActor<AEnemy>(enemyClass, FVector::ZeroVector, FRotator::ZeroRotator,param);
			if (enemy)
			{
				UE_LOG(LogTemp, Display, TEXT("SpawnUnit"));
				enemy->SpawnDefaultController();
				AEnemyController* controller = Cast<AEnemyController>(enemy->GetController());
				if (enemy->AddToSquad(this))
				{
					
					enemyArray._units.Add(enemy, controller);
					UE_LOG(LogTemp, Display, TEXT("AddUnit"));

				}
			}
		}
	}
	DeactivateAllUnits();
}

void AEnemySquad::SpawnAllUnits()
{

	for (auto& pairs : _unitPool)
	{
		for (auto& pair : pairs.Value._units)
		{
			if (IsActivatedUnit(&pair))
				continue;
			if(pair.Key->IsReadyToSpawn())
				SpawnUnit(&pair);
		}
	}




}

bool AEnemySquad::SpawnUnit(TPair< TObjectPtr<class AEnemy>, TObjectPtr<class AEnemyController>>* unit)
{
	if (IsActivatedUnit(unit))
		return false;
	//위치 설정.
	unit->Key->SetActorLocation(_spawnPoint->GetComponentLocation(), false, nullptr, ETeleportType::TeleportPhysics);
	UE_LOG(LogTemp, Error, TEXT("SpawnPoint %f %f %f"), _spawnPoint->GetComponentLocation().X, _spawnPoint->GetComponentLocation().Y, _spawnPoint->GetComponentLocation().Z);
	//UE_LOG(LogTemp, Error, TEXT("SpawnLoc %f %f %f"), unit->Key->GetActorLocation.X, unit->Key->GetActorLocation().Y, unit->Key->GetActorLocation().Z);
	//컨트롤러 빙의
	unit->Value->Possess(unit->Key);
	//활성화

	unit->Key->Spawn();


	return true;
}

void AEnemySquad::Command_Search()
{
	_squadState = ESquadState::Search;
	for (auto& PoolPair : _unitPool)
	{
		for (auto& pair : PoolPair.Value._units)
		{
			if (IsActivatedUnit(&pair))
			{

				pair.Value->RecieveTargetLoc(_targetLoc);

			}



		}
	}
}

void AEnemySquad::Command_Stationed()
{
	//만약 패트롤 패스가 있다면 패트롤패스를 제거,
	_squadState = ESquadState::Stationed;
	for (auto& PoolPair : _unitPool)
	{
		for (auto& pair : PoolPair.Value._units)
		{
			if (IsActivatedUnit(&pair))
			{

				pair.Key->GetPatrol()->SetPatrolPath(nullptr);

			}



		}
	}
}

void AEnemySquad::Command_Patrol()
{
	//가진 패트롤 패스를 넘김.
	if (_patrolPath == nullptr)
		Command_Stationed();
	_squadState = ESquadState::Patrol;
	for (auto& PoolPair : _unitPool)
	{
		for (auto& pair : PoolPair.Value._units)
		{
			if (IsActivatedUnit(&pair))
			{

				pair.Key->GetPatrol()->SetPatrolPath(_patrolPath);

			}



		}
	}
}

void AEnemySquad::Command_Attack()
{
	//
	_squadState = ESquadState::Attack;

	for (auto& PoolPair : _unitPool)
	{
		for (auto& pair : PoolPair.Value._units)
		{
			if (IsActivatedUnit(&pair))
			{

				pair.Value->RecieveTarget(_target);

			}



		}
	}
}

void AEnemySquad::Command_Deactivate()
{
	_squadState = ESquadState::Deactivate;
}

TPair< TObjectPtr<class AEnemy>, TObjectPtr<class AEnemyController>>* AEnemySquad::GetUnitFromPool(TSubclassOf<AEnemy> EnemyClass)
{
	if (!_unitPool.Contains(EnemyClass)) return nullptr;

	auto& Pool = _unitPool[EnemyClass]._units;
	for (auto& Enemy : Pool)
	{
		if (!IsActivatedUnit(&Enemy)) // 비활성화 상태인 경우
		{
			SpawnUnit(&Enemy);


			return &Enemy;
		}
	}
	return nullptr; // 사용 가능한 유닛 없음
}

void AEnemySquad::ReturnUnitToPool(AEnemy* enemy)
{

	if (!enemy) return;
	

	enemy->SetActorHiddenInGame(true);
	enemy->SetActorEnableCollision(false);
	enemy->SetActorTickEnabled(false);

	// 위치 초기화는 필요에 따라
	enemy->SetActorLocation(FVector::ZeroVector);

	AEnemyController* controller = Cast<AEnemyController>(enemy->GetController());
	if (controller)
	{
		controller->UnPossess();
	}


}

void AEnemySquad::DeactivateAllUnits()
{
	for (auto& PoolPair : _unitPool)
	{
		for (auto&  pair : PoolPair.Value._units)
		{
			
			ReturnUnitToPool(pair.Key);



		}
	}
}

TPair< TObjectPtr<class AEnemy>, TObjectPtr<class AEnemyController>>* AEnemySquad::CheckExtraUnit()
{

	for (auto& pairs : _unitPool)
	{
		for (auto& pair : pairs.Value._units)
		{
			if (IsActivatedUnit(&pair))
			{
				continue;

			}
			if (pair.Key->IsReadyToSpawn())
				return &pair;
		}
	}

	return nullptr;
}

int32 AEnemySquad::CheckActivateUnitCount()
{
	int32 count = 0;
	for (auto& pairs : _unitPool)
	{
		for (auto& pair : pairs.Value._units)
		{
			if (IsActivatedUnit(&pair))
			{
				continue;

			}
			
			count++;
		}
	}


	return count;
}

bool AEnemySquad::IsActivatedUnit(TPair< TObjectPtr<class AEnemy>, TObjectPtr<class AEnemyController>>* unit)
{
	//유닛의 폰이 컨트롤러가 없거나 컨트롤러가 폰이 없다면 그건 해제되어있는 상태. 
	if (unit->Key->GetController() == nullptr || unit->Value->GetPawn() == nullptr)
		return false;

	return true;
}

