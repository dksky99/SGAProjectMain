// Fill out your copyright notice in the Description page of Project Settings.


#include "BT_Task_MoveToTarget.h"

#include "../../../Controller/EnemyController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../../CharacterBase.h"
UBT_Task_MoveToTarget::UBT_Task_MoveToTarget()
{
	bNotifyTick = true;
}

EBTNodeResult::Type UBT_Task_MoveToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type result = Super::ExecuteTask(OwnerComp, NodeMemory);

	//현재 빙의된 Pawn 찾기
	auto currentPawn = Cast<ACharacterBase>(OwnerComp.GetAIOwner()->GetPawn());

	AActor* remain = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName(TEXT("Target"))));
	//실패 반환
	if (currentPawn->IsValidLowLevel() == false)
		return EBTNodeResult::Failed;

	//if (currentPawn->IsAttack())
	//	return EBTNodeResult::Failed;
	if (remain->IsValidLowLevel() == false)
		return EBTNodeResult::Failed;



	OwnerComp.GetAIOwner()->MoveToActor(remain, 20.0f, true);



	return EBTNodeResult::InProgress;
}

void UBT_Task_MoveToTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!AIController || !BlackboardComp)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	//현재 빙의된 Pawn 찾기
	auto currentPawn = Cast<ACharacterBase>(OwnerComp.GetAIOwner()->GetPawn());

	AActor* remain = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName(TEXT("Target"))));
	//실패 반환
	if (currentPawn->IsValidLowLevel() == false || remain->IsValidLowLevel() == false)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	//
	//if (currentPawn->IsAttack())
	//{
	//
	//	FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	//	return;
	//}
	//float Distance = FVector::Dist(remain->GetActorLocation(), currentPawn->GetActorLocation());
	//
	//if (Distance <= currentPawn->GetAttackRange() * 0.8f)
	//{
	//	AIController->StopMovement();
	//	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	//}
}
