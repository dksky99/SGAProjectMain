// Fill out your copyright notice in the Description page of Project Settings.


#include "BT_Task_FarDistance.h"

#include "../../Enemy.h"
#include "../../EnemySquad.h"
#include "../../../CharacterBase.h"

#include "../../../../Controller/EnemyController.h"

#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BehaviorTree.h"

UBT_Task_FarDistance::UBT_Task_FarDistance()
{
	bNotifyTick = true; // 틱 쓰기 + 컨트롤러에 알림 전달하기
	NodeName = "FarActing";
}

EBTNodeResult::Type UBT_Task_FarDistance::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	Super::ExecuteTask(OwnerComp, NodeMemory);

	return EBTNodeResult::Failed;    // 공격 "행동" 진행 중
}

void UBT_Task_FarDistance::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
}
