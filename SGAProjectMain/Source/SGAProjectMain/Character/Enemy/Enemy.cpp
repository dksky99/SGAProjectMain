// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"

#include "../CharacterAnimInstance.h"
#include "../CharacterStateComponent.h"
#include "Components/WidgetComponent.h"
#include "../../UI/DummyHpBar.h"
#include "../StatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../Controller/EnemyController.h"
#include "EnemySquad.h"
#include "PatrolComponent.h"

#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Damage.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"      // GetWorld, Overlap 함수들
#include "Engine/OverlapResult.h"

#include "BehaviorControlComponent.h"

AEnemy::AEnemy(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    Super::_stateComp = CreateDefaultSubobject<UCharacterStateComponent>(TEXT("State"));

    _hpBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HpBar"));
    _hpBarWidget->SetupAttachment(GetMesh());
    _hpBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
    _hpBarWidget->SetRelativeLocation(FVector(0, 0, 230.0f));

    _patrolComponent=CreateDefaultSubobject<UPatrolComponent>(TEXT("Patrol"));
    SetGenericTeamId(FGenericTeamId((int32)ETeamID::Enemy));
}

void AEnemy::BeginPlay()
{
    Super::BeginPlay();



    auto hpBar = Cast<UDummyHpBar>(_hpBarWidget->GetWidget());
    if (hpBar)
    {
       // _statComponent->_enemyHpChanged.AddUObject(hpBar, &UDummyHpBar::SetHp);
        _statComponent->_coreHpChanged.AddUObject(hpBar, &UDummyHpBar::SetHp);
    }
}

void AEnemy::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    //UE_LOG(LogTemp, Display, TEXT("%s PossesedBy :%s"), *(this->GetName()), *(NewController->GetName()));

}

void AEnemy::UnPossessed()
{
    Super::UnPossessed();

    UE_LOG(LogTemp, Display, TEXT("%s UnPossessed"), *(this->GetName()));

}

void AEnemy::FoundTarget(ACharacterBase* target)
{
}

void AEnemy::RaiseAlert()
{
    
    if (GetController() == nullptr)
        return;
    AEnemyController* temp = Cast<AEnemyController>(GetController());
    if (temp == nullptr)
        return;


    FVector Center = GetActorLocation();   // 자신 위치
    float Radius = 1000.f;                 // 탐색 반경
    FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);

    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this); // 자기 자신 제외

    // Pawn 채널만 검사하는 예시
    bool bHit = GetWorld()->OverlapMultiByChannel(
        Overlaps,
        Center,
        FQuat::Identity,
        ECC_Pawn,  // 원하는 채널
        Sphere,
        QueryParams
    );

    if (bHit)
    {
        for (auto& overlap : Overlaps)
        {
            AEnemy* target = Cast<AEnemy>(overlap.GetActor());
            if (target)
            {
                target->RecieveAlert(temp->GetBehaviorControl()->GetTargetActor());
            }

        }
    }

    UCharacterAnimInstance* anim = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());


    if (_warCryMontage==nullptr)
        return ;
    if (anim == nullptr)
        return ;
    if (_stateComp->ActionBegin() == false)
        return ;



    const float Duration = anim->PlayAnimMontage(_warCryMontage);
    if (Duration <= 0.0f)
    {

        return ;
    }


    


}

void AEnemy::RecieveAlert(AActor* target)
{
    if (GetController() == nullptr)
        return;
    AEnemyController* temp = Cast<AEnemyController>(GetController());
    if (temp == nullptr)
        return;
    temp->RecieveTargetLoc(target->GetActorLocation());

}

bool AEnemy::AddToSquad(AEnemySquad* squad)
{
    if (squad == nullptr)
        return false;
    _squad = squad;
    if (_squad == nullptr)
        return false;
    return true;
}

void AEnemy::Dead()
{
    Super::Dead();


    AController* CurrentController = GetController();
    if (CurrentController)
    {
        CurrentController->UnPossess();
    }

    GetWorldTimerManager().SetTimer(_knockDownTimerHandle, this, &ACharacterBase::RecoverFromDead, 60.0f, false);
}

void AEnemy::SpawnGhost()
{
    Super::SpawnGhost();


}

void AEnemy::SetUnitState(EUnitState state)
{
    if (_unitState == state)
        return;
    switch (state)
    {
    case EUnitState::Stay:
        SetStay();
        break;
    case EUnitState::Patrol:
        SetPatrol();
        break;
    case EUnitState::Weak_Alert:
        SetWeak_Alert();
        break;
    case EUnitState::Strong_Alert:
        SetStrong_Alert();
        break;
    case EUnitState::InBattle:
        SetInBattle();
        break;
    case EUnitState::MAX:
        break;
    default:
        break;
    }
}

void AEnemy::SetStay()
{
    _unitState = EUnitState::Stay;
    GetCharacterMovement()->MaxWalkSpeed = 200.0f;
   



}

void AEnemy::SetPatrol()
{
    _unitState = EUnitState::Patrol;
    GetCharacterMovement()->MaxWalkSpeed = 200.0f;
}

void AEnemy::SetWeak_Alert()
{
    _unitState = EUnitState::Weak_Alert;

    GetCharacterMovement()->MaxWalkSpeed = _statComponent->GetDefaultSpeed();
}

void AEnemy::SetStrong_Alert()
{
    _unitState = EUnitState::Strong_Alert;
    GetCharacterMovement()->MaxWalkSpeed = _statComponent->GetDefaultSpeed();
}

void AEnemy::SetInBattle()
{
    _unitState = EUnitState::InBattle;
    GetCharacterMovement()->MaxWalkSpeed = _statComponent->GetDefaultSpeed();

    RaiseAlert();

}



//float AEnemy::TakeDamage(float damageAmount, FDamageEvent const& damageEvent, AController* eventInstigator, AActor* damageCauser)
//{
//	_statComponent->ChangeHp(-damageAmount);
//
//	UE_LOG(LogTemp, Log, TEXT("Damage : %f"), damageAmount);
//
//	if (_statComponent->IsDead())
//		Dead();
//
//	return 0.0f;
//}
