// Fill out your copyright notice in the Description page of Project Settings.


#include "BT_Task_MoveToDynamicTargetLoc.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"

UBT_Task_MoveToDynamicTargetLoc::UBT_Task_MoveToDynamicTargetLoc()
{
    NodeName = "Move To Dynamic Target";

    // Blackboard 키로 Vector 값만 받도록 설정
    BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBT_Task_MoveToDynamicTargetLoc, BlackboardKey));

    AcceptableRadius = 50.0f;
}
EBTNodeResult::Type UBT_Task_MoveToDynamicTargetLoc::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

    if (!AIController || !BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }

    // 블랙보드에서 목표 위치 가져오기
    const FVector TargetLocation = BlackboardComp->GetValueAsVector(GetSelectedBlackboardKey());

    // 유효하지 않은 위치(0,0,0 등)이면 실패 처리
    if (TargetLocation == FAISystem::InvalidLocation)
    {
        return EBTNodeResult::Failed;
    }


    // 첫 번째 이동 요청
    EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(TargetLocation, AcceptableRadius, true, true, false, true);

    if (MoveResult == EPathFollowingRequestResult::RequestSuccessful)
    {
        // 태스크가 TickTask를 호출하도록 InProgress 반환
        return EBTNodeResult::InProgress;
    }

    return EBTNodeResult::Failed;
}

void UBT_Task_MoveToDynamicTargetLoc::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // 1. 이동 완료 여부 확인
    UPathFollowingComponent* PathComp = AIController->GetPathFollowingComponent();
    if (PathComp && (PathComp->GetStatus() == EPathFollowingStatus::Idle || PathComp->DidMoveReachGoal()))
    {

        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    // 2. 목표 위치가 변경되었는지 확인하고 이동 갱신
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (BlackboardComp)
    {
        const FVector NewTargetLocation = BlackboardComp->GetValueAsVector(GetSelectedBlackboardKey());

        // 현재 경로의 목표 지점과 블랙보드의 새 목표 지점이 다르면 이동 갱신
        // (약간의 오차(1.0f) 허용)
        if (!PathComp->GetPathDestination().Equals(NewTargetLocation, 1.0f))
        {
            AIController->MoveToLocation(NewTargetLocation, AcceptableRadius, true, true, false, true);

        }
    }
}

EBTNodeResult::Type UBT_Task_MoveToDynamicTargetLoc::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (AIController)
    {
        // 이동 중단
        AIController->StopMovement();

       
    }

    return EBTNodeResult::Aborted;
}

void UBT_Task_MoveToDynamicTargetLoc::CheckMoveCompletion(UBehaviorTreeComponent& OwnerComp)
{
}
