// Fill out your copyright notice in the Description page of Project Settings.


#include "BT_Task_SurroudingMove.h"
#include "AIController.h"
#include "NavigationSystem.h" // 내비게이션 시스템 사용을 위해 필요
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h" // GetCharacter() 사용을 위해
#include "Blueprint/AIBlueprintHelperLibrary.h"

EBTNodeResult::Type UBT_Task_SurroudingMove::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    ACharacter* AIPawn = AIController->GetCharacter();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

    if (!AIController || !AIPawn || !BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }

    AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
    if (!TargetActor)
    {
        return EBTNodeResult::Failed;
    }

    const FVector TargetLocation = TargetActor->GetActorLocation();
    const FVector TargetRightVector = TargetActor->GetActorRightVector();
    const float Direction = FMath::RandBool() ? 1.0f : -1.0f; 
    const FVector IdealFlankLocation = TargetLocation + (TargetRightVector * FlankDistance * Direction);

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys)
    {
        return EBTNodeResult::Failed;
    }

    FNavLocation ReachableFlankLocation;
    bool bFoundReachablePoint = NavSys->GetRandomReachablePointInRadius(IdealFlankLocation, SearchRadius, ReachableFlankLocation);

    if (!bFoundReachablePoint)
    {
        return EBTNodeResult::Failed;
    }

    AIController->SetFocus(TargetActor);

    UAIBlueprintHelperLibrary::SimpleMoveToLocation(AIController, ReachableFlankLocation.Location);

    return EBTNodeResult::InProgress;
}

void UBT_Task_SurroudingMove::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{

}

void UBT_Task_SurroudingMove::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

    AAIController* AIController = OwnerComp.GetAIOwner();
    if (AIController)
    {
        AIController->ClearFocus(EAIFocusPriority::Gameplay);
    }
}
