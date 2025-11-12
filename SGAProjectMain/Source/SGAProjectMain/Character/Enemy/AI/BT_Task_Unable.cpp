// Fill out your copyright notice in the Description page of Project Settings.


#include "BT_Task_Unable.h"

#include "../Enemy.h"
#include "../EnemySquad.h"
#include "../../CharacterBase.h"
#include "../../CharacterStateComponent.h"

#include "../../../Controller/EnemyController.h"

#include "../BehaviorControlComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BehaviorTree.h"

UBT_Task_Unable::UBT_Task_Unable()
{
	bNotifyTick = true; // 틱 쓰기 + 컨트롤러에 알림 전달하기
	NodeName = "Unable";
}

EBTNodeResult::Type UBT_Task_Unable::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	Super::ExecuteTask(OwnerComp, NodeMemory);


	AEnemyController* controller = Cast<AEnemyController>(OwnerComp.GetOwner());

	AEnemy* selfRef = Cast<AEnemy>(controller->GetPawn()); 
	if (selfRef->GetStateComponent()->IsUnable() == false)	//상태가 Unable이 아닐경우 무시
	{

		return EBTNodeResult::Failed;   
	}

	return EBTNodeResult::InProgress;   
}

void UBT_Task_Unable::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{


	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	AEnemyController* controller = Cast<AEnemyController>(OwnerComp.GetOwner());

	AEnemy* selfRef = Cast<AEnemy>(controller->GetPawn()); // 컨트롤러의 주인 = NPC 본인


	if (selfRef->GetStateComponent()->IsUnable()==false) // 상태가 해제됨.
	{
		// 상태를 복구
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded); // 종료 및 결과 반환
	}

}
