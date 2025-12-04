#include "BT_Task_Patrol.h"
#include "CPatrolPath.h"
#include "../../Enemy.h"
#include "../../PatrolComponent.h"
#include "../../../../Controller/EnemyController.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../BehaviorControlComponent.h"

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
	_ownerComp = &OwnerComp;
	// 초기화 . 길을 못찾았다.
	UCharacterMovementComponent* MoveComp = selfRef->GetCharacterMovement();
	

	UBehaviorControlComponent* behavior = controller->GetBehaviorControl(); // 현재 컨트롤러의 행동
	if (patrol)
	{
		patrol->UpdateNextIndex();
	}
	// 1. 이동 목표 획득 (멤버 변수 _targetLocation에 저장)
	if (patrol->GetMoveTo(_targetLocation, _acceptance) == false)
	{
		UE_LOG(LogTemp, Display, TEXT("Get Next Loc Fail"));
		return EBTNodeResult::Failed;
	}
	
	// 패트롤시작 전 유닛의 로테이션이 캐릭터의 진행방향을 따라가게한다.
	selfRef->GetCharacterMovement()->bUseControllerDesiredRotation= false;
	selfRef->GetCharacterMovement()->bOrientRotationToMovement = true;
	
		UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSystem)
	{
		FNavLocation ProjectedLoc;
		// 산악 지형 고려: 위아래 탐색 범위(Extent)를 넉넉히 설정
		if (NavSystem->GetRandomPointInNavigableRadius(_targetLocation,  _acceptance, ProjectedLoc))
		{
			_targetLocation = ProjectedLoc.Location; // 보정된 좌표로 덮어쓰기
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Patrol Point is OFF-NAVMESH! Moving to Raw Location."));
		}
	}
	behavior->SetTargetLoc(_targetLocation);

	return EBTNodeResult::Succeeded;
	//UE_LOG(LogTemp, Warning, TEXT("%s Go To %f %f %f"),*(selfRef->GetName()),_targetLocation.X, _targetLocation.Y,_targetLocation.Z);
	//EPathFollowingRequestResult::Type type = controller->MoveToLocation(
	//	_targetLocation,
	//	_acceptance,
	//	false,
	//	true
	//);
	//
	//controller->ReceiveMoveCompleted.RemoveAll(this);
	//// [CASE 1] 길찾기 성공 (이제 AI 컨트롤러가 알아서 이동함)
	//if (type == EPathFollowingRequestResult::RequestSuccessful)
	//{
	//	bNotifyTick = false;
	//	controller->ReceiveMoveCompleted.AddDynamic(this, &UBT_Task_Patrol::OnMoveCompletedHandler);
	//
	//	return EBTNodeResult::InProgress;
	//}
	//
	//if (type == EPathFollowingRequestResult::Failed || type == EPathFollowingRequestResult::AlreadyAtGoal)
	//{
	//	// TickTask 활성화 (Force Move)
	//	bNotifyTick = true;
	//	return EBTNodeResult::InProgress;
	//}
	//
	//
	//// 4. 여기선 위치를 정하는것까지만 나머지는 Tick에서 진행.
	//return EBTNodeResult::Failed;
}

void UBT_Task_Patrol::OnMoveCompletedHandler(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{

	
	if (_ownerComp==nullptr) return;

	AEnemyController* controller = Cast<AEnemyController>(_ownerComp->GetOwner());
	if (!controller)
	{
		FinishLatentTask(*_ownerComp, EBTNodeResult::Failed);
		return;
	}

	AEnemy* selfRef = Cast<AEnemy>(controller->GetPawn());
	UPatrolComponent* patrol = selfRef ? selfRef->GetPatrol() : nullptr;


	
	controller->ReceiveMoveCompleted.RemoveAll(this);

	if (Result == EPathFollowingResult::Success)
	{
		UE_LOG(LogTemp, Display, TEXT("Patrol Success"));
		if (patrol)
		{
			patrol->UpdateNextIndex();
		}
		FinishLatentTask(*_ownerComp, EBTNodeResult::Succeeded);
	}
	else
	{
		FinishLatentTask(*_ownerComp, EBTNodeResult::Failed);
	}
}

void UBT_Task_Patrol::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	

	AEnemyController* controller = Cast<AEnemyController>(OwnerComp.GetOwner());
	if (!controller) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	AEnemy* selfRef = Cast<AEnemy>(controller->GetPawn());
	if (!selfRef) return;

	FVector CurrentLoc = selfRef->GetActorLocation();

	// 1. 수동 도착 판정
	float Dist = FVector::Dist(CurrentLoc, _targetLocation);
	if (Dist <= _acceptance ) // 약간의 오차 허용
	{
		UPatrolComponent* patrol = selfRef->GetPatrol();
		
		if (patrol)
		{
			patrol->UpdateNextIndex();
		}
		UCharacterMovementComponent* Movement = selfRef->GetCharacterMovement();
		if (Movement)
		{
			// 현재 속도를 0으로 만듭니다.
			Movement->StopMovementImmediately();
		}
		// 도착했으므로 태스크 종료
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	return;
	// 2. 물리적으로 이동 입력 (Add Movement Input)
	FVector Direction = (_targetLocation - CurrentLoc);
	Direction.Z = 0.0f;

	if (!Direction.IsZero())
	{
		Direction.Normalize();

		UE_LOG(LogTemp, Warning, TEXT("%s Go To Dir %f %f %f"), *(selfRef->GetName()), Direction.X, Direction.Y, Direction.Z);
		selfRef->AddMovementInput(Direction, 1.0f);
	}
}