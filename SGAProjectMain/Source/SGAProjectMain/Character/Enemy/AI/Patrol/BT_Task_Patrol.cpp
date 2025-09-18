
#include "BT_Task_Patrol.h"

#include "CPatrolPath.h"
#include "../../Enemy.h"
#include "../../PatrolComponent.h"
#include "../../../../Controller/EnemyController.h"
#include "Navigation/PathFollowingComponent.h"



UBT_Task_Patrol::UBT_Task_Patrol()
{
    bNotifyTick = true;
    NodeName = "Patrol";
}

EBTNodeResult::Type UBT_Task_Patrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

    AEnemyController* controller = Cast<AEnemyController>(OwnerComp.GetOwner());
    AEnemy* selfRef=Cast<AEnemy>(controller->GetPawn());
    UPatrolComponent* patrol = selfRef->GetPatrol();

    FVector location;
    float acceptance;

    if (patrol->GetMoveTo(location, acceptance) == false)
    {
        return EBTNodeResult::Failed;
    }




    return EBTNodeResult::InProgress;
}

void UBT_Task_Patrol::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);


    AEnemyController* controller = Cast<AEnemyController>(OwnerComp.GetOwner());
    AEnemy* selfRef = Cast<AEnemy>(controller->GetPawn());
    UPatrolComponent* patrol = selfRef->GetPatrol();

    FVector location;
    float acceptance;


    patrol->GetMoveTo(location, acceptance);

    EPathFollowingRequestResult::Type type =
        controller->MoveToLocation(
            location,
            acceptance,
            false
        );

    if (type == EPathFollowingRequestResult::Failed)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    if (type == EPathFollowingRequestResult::AlreadyAtGoal)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        patrol->UpdateNextIndex();
    }


}
