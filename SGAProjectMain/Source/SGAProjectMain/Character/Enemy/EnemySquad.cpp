// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySquad.h"
#include "Enemy.h"
#include "PatrolComponent.h"
#include "NavigationSystem.h"
#include "../../Controller/EnemyController.h"
#include "../StatComponent.h"

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
		

		for (int i = 0; i < count; ++i)
		{
			AEnemy* enemy = GetWorld()->SpawnActor<AEnemy>(enemyClass, FVector::ZeroVector, FRotator::ZeroRotator,param);
			if (enemy)
			{
				UE_LOG(LogTemp, Display, TEXT("SpawnUnit"));
				enemy->SpawnDefaultController();
				
				if (enemy->AddToSquad(this))
				{
					
					_unitPool.Add(enemy);
					UE_LOG(LogTemp, Display, TEXT("AddUnit"));

				}
			}
		}
	}
	DeactivateAllUnits();
}




void AEnemySquad::UnitSpawnAct(class AEnemy* unit)
{
	switch (_squadState)
	{
	case ESquadState::Stationed:
	case ESquadState::Search:
		unit->GetCachedController()->RecieveTargetLoc(_targetLoc);
		//소환 후 자신의 소환 위치 에서 일정 범위 내의 랜덤 위치로 이동.경계치를 5부여
		break;
	case ESquadState::Patrol:
		//패트롤 패스를 부여.
		break;
	case ESquadState::Attack:
		unit->GetCachedController()->RecieveTarget(_target);
		//타겟을 부여
		break;
	case ESquadState::Deactivate:
	case ESquadState::MAX:
	default:
		break;
	}
}


void AEnemySquad::Command_Search()
{
	_squadState = ESquadState::Search;

	for (auto unit : _unitPool)
	{

		if (IsActivatedUnit(unit))
		{
			unit->GetCachedController()->RecieveTargetLoc(_targetLoc);

		}
	}
	
}

void AEnemySquad::Command_Stationed()
{
	//만약 패트롤 패스가 있다면 패트롤패스를 제거,
	_squadState = ESquadState::Stationed;
	for (auto unit : _unitPool)
	{
		
		if (IsActivatedUnit(unit))
		{
			unit->GetCachedController()->RecieveTargetLoc(_targetLoc);

		}
	}
}

void AEnemySquad::Command_Patrol()
{
	//가진 패트롤 패스를 넘김.
	if (_patrolPath == nullptr)
		Command_Stationed();
	_squadState = ESquadState::Patrol;
	for (auto unit : _unitPool)
	{

		if (IsActivatedUnit(unit))
		{
			unit->GetPatrol()->SetPatrolPath(_patrolPath);

		}
	}

	
}

void AEnemySquad::Command_Attack()
{
	//
	_squadState = ESquadState::Attack;

	for (auto unit : _unitPool)
	{

		if (IsActivatedUnit(unit))
		{
			unit->GetCachedController()->RecieveTarget(_target);

		}
	}

}

void AEnemySquad::Command_Deactivate()
{
	_squadState = ESquadState::Deactivate;
}

void AEnemySquad::ReturnToSquad()
{
}

AEnemy* AEnemySquad::GetUnitFromPool(TSubclassOf<AEnemy> EnemyClass)
{

	for (auto Enemy : _unitPool)
	{
		if (Enemy->GetClass() != EnemyClass)
			continue;
		if (!IsActivatedUnit(Enemy)) // 비활성화 상태인 경우
		{
			//SpawnUnit(Enemy);


			return Enemy;
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
	for (auto unit : _unitPool)
	{
		
			ReturnUnitToPool(unit);

	}
}

class AEnemy* AEnemySquad::CheckExtraUnit()
{

	for (auto unit  : _unitPool)
	{
		if (IsActivatedUnit(unit))
		{
			continue;

		}
		if (unit->IsReadyToSpawn())
			return unit;
		
	}

	return nullptr;
}

bool AEnemySquad::IsActivatedSquad()
{
	//활성화된 유닛수가 0개라면 비활성화상태
	if(CheckActivateUnitCount()==0)
		return false;
	return true;
}

int32 AEnemySquad::CheckActivateUnitCount()
{
	int32 count = 0;
	for (auto unit : _unitPool)
	{
		if (IsActivatedUnit(unit))
		{
			continue;

		}
		
		count++;
		
	}


	return count;
}

bool AEnemySquad::IsActivatedUnit(class AEnemy* unit)
{
	//유닛의 폰이 컨트롤러가 있거나 IsDead상태라면 활성화된 유닛.
	if (unit->GetController() != nullptr||unit->GetStatComponent()->IsDead()==true)
		return false;

	return true;
}

bool AEnemySquad::IsAliveUnit(AEnemy* unit)
{
	//살아있는 유닛은 컨트롤러가 붙어있으면 살아있는 유닛. 
	if (unit->GetController() == nullptr)
		return false;
	return true;
}

FVector AEnemySquad::MakeRandomLocation()
{
	FVector pos = _targetLoc;

	//NavMesh 찾기 : 이 지점을 기준으로 특정범위내에 소환가능위치가 있는지 확인. 
	auto naviSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());

	if (naviSystem->IsValidLowLevel() == false)
		return _targetLoc;



	//반환받을 랜덤한 위치.
	FNavLocation randLocation;
	//일정 반경안의 랜덤한 지점을 가져오는 함수 여기서 가능한 위치가 없으면 false를 반환.
	if (naviSystem->GetRandomPointInNavigableRadius(pos, _targetLocRadius, randLocation))
	{


		return randLocation;
	}



	return _targetLoc;
}

