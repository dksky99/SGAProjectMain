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
#include "../../../Helper/AIActingHelperLibrary.h"

void UBT_Service_Enemy_InBattle::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// 틱에 의해 값의 갱신을 확인하고 행동 유형을 설정

	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds); // 틱 진행

	// 데이터 받아오기

	AEnemyController* controller = Cast<AEnemyController>(OwnerComp.GetOwner());
	UBehaviorControlComponent* behavior = controller->GetBehaviorControl(); // 현재 컨트롤러의 행동

	AEnemy* selfRef = Cast<AEnemy>(controller->GetPawn()); // 컨트롤러의 주인 = NPC 본인
	AActor* target = behavior->GetTargetActor();				 // 현재 행동(진행 중)의 대상 = 지금은 플레이어


	if (target == nullptr) // 표적이 없으면...
	{
		behavior->SetTargetActor(nullptr);
		behavior->ChangeBattleType(EBattleState::None); // 대상이 없으면 순찰 전환
		return;
	}


		// 여기로 왔다 = 표적이 있다

		// 2. 표적과의 거리를 볼 차례


	ITargetable* targetable = Cast<ITargetable>(target);
	FVector targetLook;
	FVector targetEye;
	float angle;
	bool bIsFacingMe = false;
	//타겟이 시선을 가지지 못한다면 중단
	if (targetable->GetTargetLook(targetEye, targetLook) )
	{

		bIsFacingMe = UAIActingHelperLibrary::IsTargetFacingMe(selfRef->GetActorLocation(), targetEye, targetLook, angle, 45.f);
		UE_LOG(LogTemp, Display, TEXT("He Look At Me %f : %s"), angle, bIsFacingMe ? TEXT("true") : TEXT("false"));
		DrawDebugLine(
			GetWorld(),                 // 월드 컨텍스트
			targetEye,                  // 시작점 (특정 위치)
			targetEye + targetLook * 500.f,                    // 끝점 (시작점 + 방향 * 길이)
			FColor::Red,                // 라인 색상 (빨간색으로 설정)
			false,                      // 지속 여부 (false = 이 프레임에만 표시)
			0.1f,                       // 지속 시간 (0.1초 동안 유지)
			0,                          // 깊이 그룹 (기본값)
			2.0f                        // 라인 두께 (2.0으로 설정)
		);

	}
	
	behavior->SetIsTargetLookAtMe(bIsFacingMe);


	float distance = selfRef->GetDistanceTo(target);// 나로부터 상대까지의 위치 간 거리

	behavior->ChangeDistanceValue(distance);
	// 3. 이 거리로 곧장 행동 계산을 진행

	if (distance <= selfRef->GetMeleeRange()) // 공격 범위 안이라면
	{
		behavior->ChangeBattleType(EBattleState::Melee);
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Blue, TEXT("Melee"));
		return;
	}
	if (distance <= selfRef->GetNearRange()) // 공격 범위 안이라면
	{
		behavior->ChangeBattleType(EBattleState::Near);

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Blue, TEXT("Near"));
		return;
	}

	if (distance <= selfRef->GetMiddleRange()) // 공격 범위 안이라면
	{
		behavior->ChangeBattleType(EBattleState::Middle);
		behavior->SetIsAbleToAct(selfRef->CheckAbleTryMiddle(target));

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Blue, TEXT("Middle"));
		return;
	}
	if (distance <= selfRef->GetFarRange()) // 공격 범위 안이라면
	{
		behavior->ChangeBattleType(EBattleState::Far);
		behavior->SetIsAbleToAct(selfRef->CheckAbleTryFar(target));

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Blue, TEXT("Far"));
		return;
	}

	behavior->ChangeBattleType(EBattleState::OutOfRange);

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Blue, TEXT("OutOfRange"));

}
