// Fill out your copyright notice in the Description page of Project Settings.


#include "BT_Task_FarDistance.h"

#include "../../Enemy.h"
#include "../../EnemySquad.h"
#include "../../../CharacterBase.h"
#include "../../../CharacterStateComponent.h"

#include "../../../../Controller/EnemyController.h"

#include "../../BehaviorControlComponent.h"
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


	AEnemyController* controller = Cast<AEnemyController>(OwnerComp.GetOwner());
	UBehaviorControlComponent* behavior = controller->GetBehaviorControl(); // 현재 컨트롤러의 행동

	AEnemy* selfRef = Cast<AEnemy>(controller->GetPawn()); // 컨트롤러의 주인 = NPC 본인
	AActor* target = behavior->GetTargetActor();

	if (selfRef->TryFar(target) == false)
	{

		return EBTNodeResult::Failed;    // 공격 "행동" 진행 중
	}

	return EBTNodeResult::InProgress;    // 공격 "행동" 진행 중

}

void UBT_Task_FarDistance::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// 공격을 계속할 것인가 + 스스로 끝낸 경우 어떻게 할 것인가

	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	AEnemyController* controller = Cast<AEnemyController>(OwnerComp.GetOwner());
	UBehaviorControlComponent* behavior = controller->GetBehaviorControl(); // 현재 컨트롤러의 행동

	AEnemy* selfRef = Cast<AEnemy>(controller->GetPawn()); // 컨트롤러의 주인 = NPC 본인
	AActor* target = behavior->GetTargetActor();


	if (!selfRef->GetStateComponent()->IsActing()) // 외부에서 이걸 false로 고칠 일이 없었는데 스스로 false?
	{
		// 공격이 "완료"된 것으로 간주 (샘플 시나리오)
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded); // 종료 및 결과 반환
	}
}
