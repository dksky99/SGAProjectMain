// Fill out your copyright notice in the Description page of Project Settings.


#include "BT_Task_Patrol_FindNextLoc.h"

#include "../../../Controller/EnemyController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

EBTNodeResult::Type UBT_Task_Patrol_FindNextLoc::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type result = Super::ExecuteTask(OwnerComp, NodeMemory);

	//현재 빙의된 Pawn 찾기
	auto currentPawn = OwnerComp.GetAIOwner()->GetPawn();

	//실패 반환
	if (currentPawn->IsValidLowLevel() == false)
		return EBTNodeResult::Failed;



	FVector pos = currentPawn->GetActorLocation();

	//NavMesh 찾기
	auto naviSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());

	if (naviSystem->IsValidLowLevel() == false)
		return EBTNodeResult::Failed;

	//반환받을 랜덤한 위치.
	FNavLocation randLocation;
	//일정 반경안의 랜덤한 지점을 가져오는 함수
	if (naviSystem->GetRandomPointInNavigableRadius(pos, 500.0f, randLocation))
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(FName(TEXT("TargetLoc")), randLocation);

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
