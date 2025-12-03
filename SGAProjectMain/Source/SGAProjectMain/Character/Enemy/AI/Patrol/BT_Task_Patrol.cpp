#include "BT_Task_Patrol.h"
#include "CPatrolPath.h"
#include "../../Enemy.h"
#include "../../PatrolComponent.h"
#include "../../../../Controller/EnemyController.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"

UBT_Task_Patrol::UBT_Task_Patrol()
{
	bNotifyTick = true;
	NodeName = "Patrol";
}

EBTNodeResult::Type UBT_Task_Patrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyController* controller = Cast<AEnemyController>(OwnerComp.GetOwner());
	if (!controller) { return EBTNodeResult::Failed; }

	AEnemy* selfRef = Cast<AEnemy>(controller->GetPawn());
	if (!selfRef) { return EBTNodeResult::Failed; }

	UPatrolComponent* patrol = selfRef->GetPatrol();
	if (!patrol) { return EBTNodeResult::Failed; }

	// 초기화
	_findSuccess = false;

	// 1. 이동 목표 획득 (멤버 변수 _targetLocation에 저장)
	if (patrol->GetMoveTo(_targetLocation, _acceptance) == false)
	{
		UE_LOG(LogTemp, Display, TEXT("Get Next Loc Fail"));
		return EBTNodeResult::Failed;
	}
	selfRef-> bUseControllerRotationYaw = false;
	selfRef->GetCharacterMovement()->bOrientRotationToMovement = true;
	// 2. 내비메시 투영 (Projection)
	// 스플라인 좌표를 땅바닥(NavMesh) 위로 보정합니다.
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSystem)
	{
		FNavLocation ProjectedLoc;
		// 산악 지형 고려: 위아래 탐색 범위(Extent)를 넉넉히 설정
		if (NavSystem->GetRandomPointInNavigableRadius(_targetLocation,  1000.f, ProjectedLoc))
		{
			_targetLocation = ProjectedLoc.Location; // 보정된 좌표로 덮어쓰기
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Patrol Point is OFF-NAVMESH! Moving to Raw Location."));
		}
	}

	// 3. 기존 델리게이트 해제 (안전장치)
	controller->ReceiveMoveCompleted.RemoveAll(this);

	// 4. MoveToLocation을 여기서 호출하지 않음.
	// 의도하신 대로 TickTask에서 처리하도록 InProgress 반환.
	return EBTNodeResult::InProgress;
}

void UBT_Task_Patrol::OnMoveCompletedHandler(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	UBehaviorTreeComponent* BehaviorComp = Cast<UBehaviorTreeComponent>(GetOuter());
	if (!BehaviorComp) return;

	AEnemyController* controller = Cast<AEnemyController>(BehaviorComp->GetOwner());
	if (!controller)
	{
		FinishLatentTask(*BehaviorComp, EBTNodeResult::Failed);
		return;
	}

	AEnemy* selfRef = Cast<AEnemy>(controller->GetPawn());
	UPatrolComponent* patrol = selfRef ? selfRef->GetPatrol() : nullptr;

	controller->ReceiveMoveCompleted.RemoveAll(this);

	if (Result == EPathFollowingResult::Success)
	{
		if (patrol)
		{
			patrol->UpdateNextIndex();
		}
		FinishLatentTask(*BehaviorComp, EBTNodeResult::Succeeded);
	}
	else
	{
		_findSuccess = false;
	}
}

void UBT_Task_Patrol::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	// 길찾기에 성공해서 이동 중이라면, 틱에서는 아무것도 하지 않음 (델리게이트 대기)
	if (_findSuccess)
	{
		return;
	}

	AEnemyController* controller = Cast<AEnemyController>(OwnerComp.GetOwner());
	if (!controller) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	AEnemy* selfRef = Cast<AEnemy>(controller->GetPawn());
	if (!selfRef) return;


	// ----------------------------------------------------------------
	// [로직] 매 프레임 NavMesh 이동 시도 -> 실패 시 강제 이동
	// ----------------------------------------------------------------

	EPathFollowingRequestResult::Type type = controller->MoveToLocation(
		_targetLocation,
		_acceptance,
		false,
		true,
		true,
		true,
		0,
		true
	);

	// [CASE 1] 길찾기 성공 (이제 AI 컨트롤러가 알아서 이동함)
	if (type == EPathFollowingRequestResult::RequestSuccessful)
	{
		_findSuccess = true; // 플래그 ON -> 이후 틱 실행 방지

		controller->ReceiveMoveCompleted.RemoveAll(this);
		controller->ReceiveMoveCompleted.AddDynamic(this, &UBT_Task_Patrol::OnMoveCompletedHandler);
		return;
	}

	// [CASE 2] 길찾기 실패 (NavMesh 끊김 등) -> 강제 이동 (Force Move)
	if (type == EPathFollowingRequestResult::Failed)
	{
		// 1. 목표 방향 계산 (수평 이동)
		FVector CurrentLoc = selfRef->GetActorLocation();
		FVector Direction = (_targetLocation - CurrentLoc);
		Direction.Z = 0.0f; // 높이차 무시하고 수평으로 힘을 가함

		if (!Direction.IsZero())
		{
			Direction.Normalize();
			// 2. 물리적으로 이동 입력
			selfRef->AddMovementInput(Direction, 1.0f);
		}

		// 3. 수동 도착 판정 (MoveToLocation이 실패했으므로 직접 거리 잰다)
		float Dist = FVector::Dist(CurrentLoc, _targetLocation);
		if (Dist <= _acceptance * 1.5f) // 약간의 오차 허용 (* 1.5)
		{
			// 도착!
			UPatrolComponent* patrol = selfRef->GetPatrol();
			if (patrol)
			{
				patrol->UpdateNextIndex();
			}
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}

	}

	// [CASE 3] 이미 목표 지점에 있음
	if (type == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		UPatrolComponent* patrol = selfRef->GetPatrol();
		if (patrol)
		{
			patrol->UpdateNextIndex();
		}
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}