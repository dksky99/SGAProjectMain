#include "HelldiverReinforceManager.h"

#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"

#include "../../Object/Stratagem/Pod/HellDiverDroppodInput.h"
#include "../../Object/Stratagem/Pod/HellDiverDropPod.h"
#include "../PlayerCharacter.h"

AHelldiverReinforceManager::AHelldiverReinforceManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AHelldiverReinforceManager::BeginPlay()
{
	Super::BeginPlay();

	// 시작 시 풀 구성(헬다이버를 미리 스폰해 두고 비활성화)
	InitSquad();

	// 예산 자동 회복(쿨타임이 양수일 때만)
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
	// 보강 가능 여부 확인
	if (_remainReinforceBudget <= 0) return;

	// 대기 중 컨트롤러 획득(없으면 복귀)
	AController* controller = nullptr;
	if (!_controllerQ.Dequeue(controller) || controller == nullptr) return;

	// 풀에서 사용 가능한 헬다이버 획득(없으면 컨트롤러 복귀)
	APlayerCharacter* pooled = AcquirePooledDiver();
	if (!pooled)
	{
		_controllerQ.Enqueue(controller);
		return;
	}

	// 사망 지점 근처의 유효 지면 좌표 계산(네비 사용)
	const FVector groundPoint = ComputeGroundPoint(deathPoint);

	// 드랍포드는 지면 위로 _dropHeight 만큼 띄운 위치에 스폰
	const FVector dropPoint = groundPoint + FVector(0.0f, 0.0f, _dropHeight);

	// 스폰에 필요한 클래스 존재 여부
	if (!_playerDropPodClass || !_dropPodInputClass)
	{
		_controllerQ.Enqueue(controller);
		return;
	}

	// 스폰 파라미터: 드랍포드 Owner를 controller로 지정
	// → 착지 후 드랍포드가 플레이어를 Possess 할 때 사용할 컨트롤러
	FActorSpawnParameters sp;
	sp.Owner = controller;
	sp.Instigator = nullptr;
	sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 드랍포드 스폰(하강 + 착지 + 플레이어 끌어올림 담당)
	AHellDiverDropPod* pod = GetWorld()->SpawnActor<AHellDiverDropPod>(_playerDropPodClass, dropPoint, FRotator::ZeroRotator, sp);
	if (!pod)
	{
		// 실패 시 컨트롤러 복귀
		_controllerQ.Enqueue(controller);
		return;
	}

	// 입력 전용 폰 스폰(낙하시 좌우 이동 입력만 담당)
	// 컨트롤러는 먼저 이 입력 폰을 Possess 한 뒤, 입력 폰이 드랍포드에 축값을 전달하는 구조
	AHellDiverDroppodInput* inputPawn = GetWorld()->SpawnActor<AHellDiverDroppodInput>(_dropPodInputClass, dropPoint, FRotator::ZeroRotator, sp);
	if (!inputPawn)
	{
		_controllerQ.Enqueue(controller);
		pod->Destroy();
		return;
	}

	// 입력 폰 포제션 및 드랍포드 연결
	controller->Possess(inputPawn);
	inputPawn->SetHellPodActor(pod);

	// 풀에서 꺼낸 헬다이버를 "새 스폰 없이" 드랍포드에 주입
	// 드랍포드는 지하에서 플레이어를 끌어올린 뒤 Owner(controller) → 플레이어로 포제션을 넘김
	pooled->UnitDeactivate();  // 안전하게 초기화(위젯/상태 등 비활성)
	pooled->ResetUnit();       // 캐릭터 내부 상태 초기화(체력/탄약 등 프로젝트 정책대로)
	pod->SetPreSpawnedPlayer(pooled);

	// 카메라: 드랍포드 탑다운 시점으로 전환(착지/소환 연출 구간에서 활용)
	pod->ActivateOverlookCamera(controller, 0.35f);

	// 예산 1 소모
	--_remainReinforceBudget;
}

void AHelldiverReinforceManager::ReturnDeadController(AController* controller)
{
	// 사망한 컨트롤러를 큐에 적재(차후 ReinforceHelldiver 호출 시 사용)
	if (!controller)
		return;

	_controllerQ.Enqueue(controller);
}

void AHelldiverReinforceManager::InitSquad()
{
	// 풀 구성: 초기 개수만큼 헬다이버를 스폰해서 비활성 상태로 저장
	if (!_hellDiverClass) return;

	FActorSpawnParameters sp;
	sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < _initialPoolSize; ++i)
	{
		APlayerCharacter* diver = GetWorld()->SpawnActor<APlayerCharacter>(_hellDiverClass, FVector::ZeroVector, FRotator::ZeroRotator, sp);
		if (!diver) continue;

		// 프로젝트 정책에 맞춰 입력/AI 오토 포제션 해제, 사용자 정의 비활성화 함수 호출
		diver->UnitDeactivate();
		diver->AutoPossessPlayer = EAutoReceiveInput::Disabled;
		diver->AutoPossessAI = EAutoPossessAI::Disabled;

		_hellDiverPool.Add(diver);
	}
}

APlayerCharacter* AHelldiverReinforceManager::AcquirePooledDiver()
{
	// 풀에서 "준비 완료" 상태의 유닛을 하나 찾아 반환
	// IsReadyToSpawn()은 프로젝트 정책에 맞춘 사용자 정의 함수(비활성/미점유/안전 상태 등 확인)
	for (APlayerCharacter* diver : _hellDiverPool)
	{
		if (diver && diver->IsReadyToSpawn())
		{
			return diver;
		}
	}
	return nullptr;
}

FVector AHelldiverReinforceManager::ComputeGroundPoint(const FVector& deathPoint) const
{
	// 네비게이션 시스템을 이용해 "이동 가능한 지면" 좌표를 구합니다.
	// 1) ProjectPointToNavigation: 입력 점을 네비 메시 위로 수직 투영(가장 근접한 유효 지점)
	// 2) 실패 시 GetRandomPointInNavigableRadius: 반경 내 무작위 유효 지점
	// 모두 실패하면 deathPoint를 그대로 돌려 안정적으로 진행합니다.
	FVector ground = deathPoint;

	if (UNavigationSystemV1* navi = UNavigationSystemV1::GetNavigationSystem(GetWorld()))
	{
		FNavLocation projected;
		if (navi->ProjectPointToNavigation(deathPoint, projected))
		{
			// 가장 가까운 네비 지점
			ground = projected.Location;
		}
		else
		{
			// 투영 실패 → 근처 반경에서 임의의 유효 지점 탐색
			FNavLocation navLoc;
			if (navi->GetRandomPointInNavigableRadius(deathPoint, _reinforceSearchRadius, navLoc))
			{
				ground = navLoc.Location;
			}
		}
	}

	return ground;
}

void AHelldiverReinforceManager::AddReinforceBudgetTick()
{
	// 주기적으로 보강 예산 1 회복(상한 관리가 필요하면 여기서 Clamp 하십시오)
	++_remainReinforceBudget;
}
