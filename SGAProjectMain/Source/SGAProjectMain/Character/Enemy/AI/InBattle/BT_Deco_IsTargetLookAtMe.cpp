// Fill out your copyright notice in the Description page of Project Settings.


#include "BT_Deco_IsTargetLookAtMe.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h" // Dot product, Acost, Radians/Degrees 변환용
#include "../../../../Helper/AIActingHelperLibrary.h"
#include "../../../../Interface/Targetable.h"

UBT_Deco_IsTargetLookAtMe::UBT_Deco_IsTargetLookAtMe()
{
    NodeName = "Is Target Looking At Me";

    // TargetActorKey는 AActor 타입의 블랙보드 키만 받도록 설정
    TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBT_Deco_IsTargetLookAtMe, TargetActorKey), AActor::StaticClass());

    // Abort Observer를 설정하여 조건이 변경될 때 트리가 재평가되도록 합니다.
    bNotifyActivation = true;
    bNotifyDeactivation = true;
    FlowAbortMode = EBTFlowAbortMode::Self; // 이 조건이 변경되면 이 분기만 다시 검사
}


bool UBT_Deco_IsTargetLookAtMe::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

    if (!AIController || !BlackboardComp)
    {
        return false;
    }

    APawn* AIPawn = AIController->GetPawn();
    if (!AIPawn)
    {
        return false;
    }

    // 1. AI(Owner)의 위치 (OwnerLoc)
    const FVector OwnerLoc = AIPawn->GetActorLocation();

    // 2. 타겟 Actor 가져오기
    AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (!TargetActor)
    {
        return false;
    }

    ITargetable* targetable = Cast<ITargetable>(TargetActor);
    FVector targetLook;
    FVector targetEye;
    float angle;

    //타겟이 시선을 가지지 못한다면 중단
    if (targetable->GetTargetLook(targetEye,targetLook) == false)
    {
       
        return false;
    }

    // 3. 타겟의 위치 및 시선 벡터 설정
    // TargetLoc: 대상 캐릭터의 위치를 시선의 시작점으로 가정

    
    // 4. 핵심 함수 호출
    const bool bIsFacingMe = UAIActingHelperLibrary::IsFacingTarget_WithAngle(OwnerLoc, targetEye, targetLook,angle, AngleTolerance);

    UE_LOG(LogTemp, Display, TEXT("He Look At Me %f : %s"),angle, bIsFacingMe ? TEXT("true") : TEXT("false"));
    DrawDebugLine(
        GetWorld(),                 // 월드 컨텍스트
        targetEye,                  // 시작점 (특정 위치)
        targetEye+targetLook*500.f,                    // 끝점 (시작점 + 방향 * 길이)
        FColor::Red,                // 라인 색상 (빨간색으로 설정)
        false,                      // 지속 여부 (false = 이 프레임에만 표시)
        0.1f,                       // 지속 시간 (0.1초 동안 유지)
        0,                          // 깊이 그룹 (기본값)
        2.0f                        // 라인 두께 (2.0으로 설정)
    );
    // IsInversed 플래그에 따라 결과를 반전
    return bIsFacingMe;
}