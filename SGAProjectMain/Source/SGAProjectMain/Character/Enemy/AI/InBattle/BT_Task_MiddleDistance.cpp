// Fill out your copyright notice in the Description page of Project Settings.


#include "BT_Task_MiddleDistance.h"

#include "../../Enemy.h"
#include "../../EnemySquad.h"
#include "../../../CharacterBase.h"

#include "../../../../Controller/EnemyController.h"

#include "../../BehaviorControlComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BehaviorTree.h"

UBT_Task_MiddleDistance::UBT_Task_MiddleDistance()
{
	bNotifyTick = true; // 틱 쓰기 + 컨트롤러에 알림 전달하기
	NodeName = "MiddleActing";
}

EBTNodeResult::Type UBT_Task_MiddleDistance::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);




	AEnemyController* controller = Cast<AEnemyController>(OwnerComp.GetOwner());
	UBehaviorControlComponent* behavior = controller->GetBehaviorControl(); // 현재 컨트롤러의 행동

	AEnemy* selfRef = Cast<AEnemy>(controller->GetPawn()); // 컨트롤러의 주인 = NPC 본인
	AActor* target = behavior->GetTargetActor();

	if (selfRef->TryMiddle(target) == false)
	{

		return EBTNodeResult::Failed;    // 공격 "행동" 진행 중
	}

	return EBTNodeResult::InProgress;    // 공격 "행동" 진행 중
}

void UBT_Task_MiddleDistance::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
}
