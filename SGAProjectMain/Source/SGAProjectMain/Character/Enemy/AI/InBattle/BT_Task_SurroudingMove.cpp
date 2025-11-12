// Fill out your copyright notice in the Description page of Project Settings.


#include "BT_Task_SurroudingMove.h"
#include "AIController.h"
#include "NavigationSystem.h" // 내비게이션 시스템 사용을 위해 필요
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h" // GetCharacter() 사용을 위해
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h" // For GetSafeNormal
#include "../../../../Helper/AIActingHelperLibrary.h"
#include "../../../../Interface/Targetable.h"

UBT_Task_SurroudingMove::UBT_Task_SurroudingMove()
{
    NodeName = "Strafe Orbit Target";
    bNotifyTick = true; // TickTask를 사용하기 위해 반드시 true로 설정
    _angleTolerence = 45.f;

    // 키 필터 설정
    TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBT_Task_SurroudingMove, TargetActorKey), AActor::StaticClass());

}

EBTNodeResult::Type UBT_Task_SurroudingMove::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    ACharacter* AIPawn = Cast<ACharacter>(AIController ? AIController->GetPawn() : nullptr);

    if (!AIController || !BlackboardComp || !AIPawn)
    {
        return EBTNodeResult::Failed;
    }

    AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (!TargetActor)
    {
        return EBTNodeResult::Failed;
    }
    
    // 1. 타겟을 바라보도록 포커스 설정
    AIController->SetFocus(TargetActor);
    
    // 2. 캐릭터가 이동 방향이 아닌 컨트롤러(시선) 방향을 보도록 설정
    UCharacterMovementComponent* MoveComp = AIPawn->GetCharacterMovement();
    bOriginalOrientRotationToMovement = MoveComp->bOrientRotationToMovement; // 원래 값 저장
    MoveComp->bOrientRotationToMovement = false;
    

    // 방향 결정
    _strafeDirection= FMath::RandBool(); 

    return EBTNodeResult::InProgress;
}

void UBT_Task_SurroudingMove::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{

    AAIController* AIController = OwnerComp.GetAIOwner();
    ACharacter* AIPawn = AIController->GetPawn() ? Cast<ACharacter>(AIController->GetPawn()) : nullptr;

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

    AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));


    if (!AIPawn || !BlackboardComp)
    {
        AbortTask(OwnerComp, NodeMemory); // 필수 컴포넌트 없으면 중단
        return;
    }

    if (!TargetActor)
    {
        AbortTask(OwnerComp, NodeMemory); // 타겟이 사라지면 중단
        return;
    }

    ITargetable* targetable = Cast<ITargetable>(TargetActor);
    FVector targetLook;
    FVector targetLoc;
    float angle=0.0f;
    //타겟이 시선을 가지지 못한다면 중단
    if (targetable->GetTargetLook(targetLoc,targetLook) == false)
    {
        AbortTask(OwnerComp, NodeMemory); // 
        return;
    }


    // 3. 타겟의 위치 및 시선 벡터 설정
    // TargetLoc: 대상 캐릭터의 위치를 시선의 시작점으로 가정


    // 4. 핵심 함수 호출
    const bool bIsFacingMe = UAIActingHelperLibrary::IsFacingTarget_WithAngle(AIPawn->GetActorLocation(), targetLoc, targetLook, angle, _angleTolerence);
    // 1. 타겟의 시선에서 벗어나면 성공 시야에서 45도이상 벗어나면 스트레이프 중단.
    if (bIsFacingMe==false)
    {
        // 설정 복원

        if (AIController)
        {
            AIController->ClearFocus(EAIFocusPriority::Gameplay);
            AIController->StopMovement(); // 이동 중지
        }
        if (AIPawn)
        {
            AIPawn->GetCharacterMovement()->bOrientRotationToMovement = bOriginalOrientRotationToMovement;
        }

        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    // 2. 스트레이프 이동 처리



    const float StrafeSign = (_strafeDirection) ? 1.0f : -1.0f;

    // AI 위치에서 타겟을 바라보는 벡터
    const FVector DirToTarget = (TargetActor->GetActorLocation() - AIPawn->GetActorLocation()).GetSafeNormal();

    // 타겟 벡터의 오른쪽(수직) 벡터 계산 (Cross Product)
    const FVector StrafeVector = FVector::CrossProduct(DirToTarget, FVector::UpVector).GetSafeNormal();

    // 최종 이동 방향 (오른쪽 또는 왼쪽)
    const FVector FinalMoveDirection = StrafeVector * StrafeSign;

    // 캐릭터 무브먼트에 이동 입력 추가
    AIPawn->AddMovementInput(FinalMoveDirection, 1.0f); // 1.0f 스케일로 최대 속도 이동
}

EBTNodeResult::Type UBT_Task_SurroudingMove::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    ACharacter* AIPawn = Cast<ACharacter>(AIController ? AIController->GetPawn() : nullptr);

    // 설정 복원
    if (AIController)
    {
        AIController->ClearFocus(EAIFocusPriority::Gameplay);
        AIController->StopMovement();
    }
    if (AIPawn)
    {
        AIPawn->GetCharacterMovement()->bOrientRotationToMovement = bOriginalOrientRotationToMovement;
    }

    return EBTNodeResult::Aborted;
}