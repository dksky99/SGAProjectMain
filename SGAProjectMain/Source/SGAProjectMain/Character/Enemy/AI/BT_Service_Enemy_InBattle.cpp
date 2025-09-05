// Fill out your copyright notice in the Description page of Project Settings.


#include "BT_Service_Enemy_InBattle.h"

#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BehaviorTree.h"

#include "../Enemy.h"
#include "../../../Controller/EnemyController.h"

#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Algo/Sort.h"
#include "../BehaviorControlComponent.h"
#include "../../../Interface/Targetable.h"

#include "../../../Helper/H_Targetting.h"

void UBT_Service_Enemy_InBattle::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// 틱에 의해 값의 갱신을 확인하고 행동 유형을 설정

	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds); // 틱 진행

	// 데이터 받아오기

	AEnemyController* controller = Cast<AEnemyController>(OwnerComp.GetOwner());
	UBehaviorControlComponent* behavior = controller->GetBehaviorControl(); // 현재 컨트롤러의 행동

	ACharacterBase* selfRef = Cast<ACharacterBase>(controller->GetPawn()); // 컨트롤러의 주인 = NPC 본인
	AActor* target = behavior->GetTargetActor();				 // 현재 행동(진행 중)의 대상 = 지금은 플레이어

	UE_LOG(LogTemp, Display, TEXT("%s Find %s"), *selfRef->GetName(), *target->GetName());
	if (target == nullptr) // 표적이 없으면...
	{
		behavior->ChangeBattleType(EBattleState::None); // 대상이 없으면 순찰 전환
		return;
	}


		// 여기로 왔다 = 표적이 있다

		// 2. 표적과의 거리를 볼 차례

	float distance = selfRef->GetDistanceTo(target);// 나로부터 상대까지의 위치 간 거리

	behavior->ChangeDistanceValue(distance);
	// 3. 이 거리로 곧장 행동 계산을 진행

	if (distance <= 150) // 공격 범위 안이라면
	{
		behavior->ChangeBattleType(EBattleState::Near);
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Blue, TEXT("Near"));
		return;
	}

	if (distance <= 600) // 공격 범위 안이라면
	{
		behavior->ChangeBattleType(EBattleState::Middle);
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Blue, TEXT("Middle"));
		return;
	}

	behavior->ChangeBattleType(EBattleState::Far);

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Blue, TEXT("Far"));

}
