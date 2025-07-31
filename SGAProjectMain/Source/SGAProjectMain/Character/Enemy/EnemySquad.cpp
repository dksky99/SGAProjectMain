// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySquad.h"
#include "Enemy.h"
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
			if (!IsActivatedUnit(&pair))
			{
				SpawnUnit(&pair);

			}
		}
	}




}

bool AEnemySquad::SpawnUnit(TPair< TObjectPtr<class AEnemy>, TObjectPtr<class AEnemyController>>* unit)
{
	if (IsActivatedUnit(unit))
		return false;
	//위치 설정.
	unit->Key->SetActorLocation(_spawnPoint->GetComponentLocation());
	//컨트롤러 빙의
	unit->Value->Possess(unit->Key);
	//활성화

	unit->Key->ResetUnit();

	return true;
}

void AEnemySquad::Command_Search()
{
	for (auto& pairs : _unitPool)
	{
		for (auto& pair : pairs.Value._units)
		{
			if (IsActivatedUnit(&pair))
			{


			}
		}
	}
}

void AEnemySquad::Command_Stationed()
{

}

void AEnemySquad::Command_Patrol()
{
}

void AEnemySquad::Command_Attack()
{
}

void AEnemySquad::Command_Deactivate()
{
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
			if (!IsActivatedUnit(&pair))
			{
				return &pair;

			}
		}
	}

	return nullptr;
}

bool AEnemySquad::IsActivatedUnit(TPair< TObjectPtr<class AEnemy>, TObjectPtr<class AEnemyController>>* unit)
{
	if (unit->Key->GetController() == nullptr || unit->Value->GetPawn() == nullptr)
		return false;

	return true;
}

