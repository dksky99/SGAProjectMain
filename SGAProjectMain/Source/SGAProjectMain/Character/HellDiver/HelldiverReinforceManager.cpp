#include "HelldiverReinforceManager.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

#include "GameFramework/Controller.h"
#include "../../Object/Stratagem/Pod/HellDiverDropPod.h"

// 생성자
AHelldiverReinforceManager::AHelldiverReinforceManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AHelldiverReinforceManager::BeginPlay()
{
	Super::BeginPlay();

	// 예산 자동 회복 타이머를 시작합니다(필요 없으면 _addReinforceBudgetCoolTime을 0으로 두거나 타이머 설정을 제거하십시오)
	if (_addReinforceBudgetCoolTime > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			_reinforceBudgetTimer,
			this,
			&AHelldiverReinforceManager::AddReinforceBudgetTick,
			_addReinforceBudgetCoolTime,
			true
		);
	}
}

void AHelldiverReinforceManager::ReinforceHelldiver(const FVector& deathPoint)
{
	// 예산 확인
	if (_remainReinforceBudget <= 0)
		return;

	// 컨트롤러 큐에서 하나를 가져옵니다
	AController* controller = nullptr;
	if (!_controllerQ.Dequeue(controller) || controller == nullptr)
		return;

	// 드랍포드 클래스 확인
	if (!_playerDropPodClass)
		return;

	// 네비게이션 시스템 참조
	FVector groundPoint = deathPoint;
	if (UNavigationSystemV1* navi = UNavigationSystemV1::GetNavigationSystem(GetWorld()))
	{
		FNavLocation navLoc;
		if (navi->GetRandomPointInNavigableRadius(deathPoint, _reinforceSearchRadius, navLoc))
		{
			groundPoint = navLoc.Location;
		}
	}

	// 드랍 포드 스폰 위치: 네비 위치(또는 사망 지점) 위쪽으로 올립니다
	const FVector dropPoint = groundPoint + FVector(0.0f, 0.0f, _dropHeight);

	// 스폰 파라미터(Owner를 컨트롤러로 지정해 드랍포드 내부 인계에 사용)
	FActorSpawnParameters sp;
	sp.Owner = controller;
	sp.Instigator = nullptr;
	sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AHellDiverDropPod* spawnedPod =
		GetWorld()->SpawnActor<AHellDiverDropPod>(_playerDropPodClass, dropPoint, FRotator::ZeroRotator, sp);

	if (!spawnedPod)
	{
		UE_LOG(LogTemp, Warning, TEXT("Reinforce failed: DropPod spawn failed"));
		// 실패했으므로 컨트롤러를 다시 큐에 되돌려 놓는 것이 안전합니다
		_controllerQ.Enqueue(controller);
		return;
	}

	// 성공: 예산 1 소모
	--_remainReinforceBudget;
}

// 사망 컨트롤러 반환
void AHelldiverReinforceManager::ReturnDeadController(AController* controller)
{
	if (!controller)
		return;

	_controllerQ.Enqueue(controller);
}

// 예산 회복 틱
void AHelldiverReinforceManager::AddReinforceBudgetTick()
{
	++_remainReinforceBudget;
}
