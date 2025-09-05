// Fill out your copyright notice in the Description page of Project Settings.


#include "BT_Task_NearDistance.h"

#include "../../Enemy.h"
#include "../../EnemySquad.h"
#include "../../../CharacterBase.h"

#include "../../../../Controller/EnemyController.h"

#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BehaviorTree.h"



UBT_Task_NearDistance::UBT_Task_NearDistance()
{

	bNotifyTick = true; // 틱 쓰기 + 컨트롤러에 알림 전달하기
	NodeName = "NearActing";

}

EBTNodeResult::Type UBT_Task_NearDistance::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	Super::ExecuteTask(OwnerComp, NodeMemory);

	//// 컨트롤러와 유닛 기능에 접근해서 공격을 "실행"
	//AEnemyController* controller = Cast<AEnemyController>(OwnerComp.GetOwner());
	//UCBehaviourComponent* behavior = controller->GetBehavior();
	//
	//ACUnit* selfRef = Cast<ACUnit>(controller->GetPawn());
	//ACUnit* target = behavior->GetPlayer();
	//
	//if (target == nullptr) // 간소화를 위한 선 예외 처리
	//	return EBTNodeResult::Failed;	// 표적 유실 = 공격 실패 (더는 행동하지 않음)
	//
	//// 여기까지 오면, 현재 표적이 확인된 상태 (그리고 거리도 충분한 상태 = 아니라면 실행도 안 됐을 테니까!)
	//
	//// -> 공격을 그냥 실행만 하면 되는 상황
	//FRotator rotation = UKismetMathLibrary::FindLookAtRotation( // "바라보기"
	//	selfRef->GetActorLocation(),							// 어디서?
	//	target->GetActorLocation()								// 어디로?
	//);
	//
	//selfRef->SetActorRotation(rotation.Quaternion()); // 로테이터를 본래의 행렬로 바꾸어 회전 적용
	//
	//// 방향도 맞춰졌으니 공격 시작
	//
	//selfRef->GetCurWeapon()	// (미리 작성된) 아이템 호출
	//	->Attack();			// 공격 실행
	//
	// return EBTNodeResult::Succeeded;  // 공격 "시도" 성공
	return EBTNodeResult::Failed;    // 공격 "행동" 진행 중
}

void UBT_Task_NearDistance::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// 공격을 계속할 것인가 + 스스로 끝낸 경우 어떻게 할 것인가

	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	//ACMonsterAIController* controller = Cast<ACMonsterAIController>(OwnerComp.GetOwner());
	//UCBehaviourComponent* behavior = controller->GetBehavior();
	//
	//ACUnit* selfRef = Cast<ACUnit>(controller->GetPawn());
	//ACUnit* target = behavior->GetPlayer();
	//
	// (현재) 공격이 자기 행동으로써 멈춰지면 성공인 것으로 간주

	// 내 행동이 외부 지시보다 먼저 "공격 중단"으로 보고되면 된다
	//if (!selfRef->GetCurWeapon()->IsAttacking()) // 외부에서 이걸 false로 고칠 일이 없었는데 스스로 false?
	{
		// 공격이 "완료"된 것으로 간주 (샘플 시나리오)
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded); // 종료 및 결과 반환
	}
}
