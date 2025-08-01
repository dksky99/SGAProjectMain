// Fill out your copyright notice in the Description page of Project Settings.


#include "BT_Service_Enemy.h"

#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BehaviorTree.h"

#include "../Enemy.h"
#include "../../../Controller/EnemyController.h"

#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Algo/Sort.h"

#include "../../../Helper/H_Targetting.h"
void UBT_Service_Enemy::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{


	bool reset=false;

	//UE_LOG(LogTemp, Display, TEXT("ServiceTest : EnemyService"));

	//현재 스테이트 값 세팅


	//Todo  : 스쿼드로부터 적이 발견되거나 상태가 변경되었을경우

	


	//적이 발견되었을 경우



	if (reset)
		OwnerComp.RestartTree();

	return;

}

void UBT_Service_Enemy::SetCurrentState()
{
}

bool UBT_Service_Enemy::SightSearch(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{

	return false;
}


