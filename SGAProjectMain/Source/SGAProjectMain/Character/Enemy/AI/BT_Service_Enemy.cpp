// Fill out your copyright notice in the Description page of Project Settings.


#include "BT_Service_Enemy.h"

#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BehaviorTree.h"

#include "../Enemy.h"
#include "../../CharacterStateComponent.h"
#include "../../../Controller/EnemyController.h"
#include "../PatrolComponent.h"

#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Algo/Sort.h"
#include "../BehaviorControlComponent.h"
#include "../../../Interface/Targetable.h"

#include "../../../Helper/H_Targetting.h"
void UBT_Service_Enemy::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{

	// 틱에 의해 값의 갱신을 확인하고 행동 유형을 설정

	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds); // 틱 진행

	// 데이터 받아오기

	AEnemyController* controller = Cast<AEnemyController>(OwnerComp.GetOwner());
	UBehaviorControlComponent* behavior = controller->GetBehaviorControl(); // 현재 컨트롤러의 행동
	
	AEnemy* selfRef = Cast<AEnemy>(controller->GetPawn()); // 컨트롤러의 주인 = NPC 본인
	
	if (behavior == nullptr)
		return;
	AActor* target = behavior->GetTargetActor();				 // 현재 행동(진행 중)의 대상 = 지금은 플레이어
	
	behavior->SetIsUnable(selfRef->GetStateComponent()->IsUnable());

	if (target) // 표적이 있으면
	{
		behavior->SetTargetActor(target);

		behavior->ChangeUnitType(EUnitState::InBattle); //전투중
		return;
	}

	behavior->SetTargetActor(nullptr);
	float threashold = behavior->GetAlertThreshold();
	behavior->ChangeAlertThreshold(threashold);
	if (threashold >= 5.0f)
	{
		behavior->ChangeUnitType(EUnitState::Strong_Alert); // 대상이 없지만 임계치가 일정이상 올라간다면 강한 경계

		behavior->SetTargetLoc(behavior->GetTargetLoc());
		return;

	}


	if (threashold >= 1.0f)
	{
		behavior->ChangeUnitType(EUnitState::Weak_Alert); // 대상이 없지만 한번이라도 소리를 듣는다면 약한경계

		behavior->SetTargetLoc(behavior->GetTargetLoc());
		return;

	}

	if (selfRef->GetPatrol()->HasPath())
	{
		behavior->ChangeUnitType(EUnitState::Patrol); 

		return;
	}
	

	
	// 가진 순찰경로도 없다면 그자리에서 대기.
	behavior->ChangeUnitType(EUnitState::Stay); 
	
}
