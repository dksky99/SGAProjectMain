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
		unit->GetPatrol()->SetPatrolPath(_patrolPath);
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


void AEnemySquad::Command_Search(FVector targetLoc)
{
	_targetLoc = targetLoc;
	
	_squadState = ESquadState::Search;

	for (auto unit : _unitPool)
	{

		
		unit->GetCachedController()->RecieveTargetLoc(_targetLoc);

		
	}
	
}

void AEnemySquad::Command_Stationed(FVector targetLoc)
{
	if (targetLoc != FVector::ZeroVector)
		_targetLoc = targetLoc;
	_squadState = ESquadState::Stationed;
	for (auto unit : _unitPool)
	{
		
		unit->GetPatrol()->SetPatrolPath(nullptr);
		//기지에서 소환된 유닛이 어찌해야할지 고민해야할듯하다. 
		//기지에서 소환된 유닛은 패트롤패스를 지워 아무일없을때 그자리에서 대기한다. 
		//소환된 유닛은 집결지로 지정된 장소에 모이며 미리 경계치를 부여받아 인근에서 소리가 들리면 바로 확인하러간다.
		unit->GetCachedController()->RecieveTargetLoc(_targetLoc);

		
	}
}

void AEnemySquad::Command_Patrol(class ACPatrolPath* path)
{
	_patrolPath = path;
	//가진 패트롤 패스를 넘김.
	if (_patrolPath == nullptr)
	{
		Command_Stationed();
		return;
	}
	_squadState = ESquadState::Patrol;
	for (auto unit : _unitPool)
	{

		unit->GetPatrol()->SetPatrolPath(_patrolPath);
		
	}

	
}

void AEnemySquad::Command_Attack(AActor* target)
{
	_target = target;
	if(_target==nullptr)
	{
		Command_Stationed();
		return;
	}
	_squadState = ESquadState::Attack;

	for (auto unit : _unitPool)
	{

		unit->GetCachedController()->RecieveTarget(_target);


	}

}

void AEnemySquad::Command_Deactivate()
{
	_squadState = ESquadState::Deactivate;
}

//병력 복귀. 살아있는 병력은 다시 땅속으로 들어가 재호출을 기다린다. 이렇게되면 이미 죽은 병력만 다시 되살아나면 소환이 가능.
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
		if (unit->IsReadyToSpawn())
		{
			return unit;

		}
		
	}

	return nullptr;
}

bool AEnemySquad::IsCallableSquad()
{
	if(IsActivatedSquad())
		return false;
	
	return true;
}

bool AEnemySquad::IsActivatedSquad()
{
	//유닛중 하나만이라도 활성화상태거나 죽어있는사태라면 소환이 불가하다.
	//증원부대만 사용. 

	if (_unitPool.IsEmpty())
		return false;
	for (auto unit : _unitPool)
	{
		if(unit->IsReadyToSpawn()==false)
			return false;
	}
	return true;
}

int32 AEnemySquad::CheckActivateUnitCount()
{
	int32 count = 0;
	for (auto unit : _unitPool)
	{
		if (IsActivatedUnit(unit))
		{

			count++;

		}
		
		
	}


	return count;
}

bool AEnemySquad::IsActivatedUnit(class AEnemy* unit)
{
	//유닛의 폰이 컨트롤러가 있거나 IsDead상태라면 활성화된 유닛.
	

	return unit->IsReadyToSpawn();
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

