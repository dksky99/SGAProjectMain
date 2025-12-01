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

#include "NavigationInvokerComponent.h"


AEnemy::AEnemy(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    Super::_stateComp = CreateDefaultSubobject<UCharacterStateComponent>(TEXT("State"));

    _hpBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HpBar"));
    _hpBarWidget->SetupAttachment(GetMesh());
    _hpBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
    _hpBarWidget->SetRelativeLocation(FVector(0, 0, 230.0f));

    _patrolComponent=CreateDefaultSubobject<UPatrolComponent>(TEXT("Patrol"));
    _navInvokerComponent = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("NavInvoker"));

    _behaviorControlComponent = CreateDefaultSubobject<UBehaviorControlComponent>("Behavior Control");
    //Tile Generation Radius(타일 생성 반경) : NavMesh를 생성할 플레이어 주변의 반경을 설정합니다. (예 : 5000 또는 10000 cm)
    //
    //    Tile Removal Radius
    _navInvokerComponent->SetGenerationRadii(2500  ,4000);
    SetGenericTeamId(FGenericTeamId((int32)ETeamID::Enemy));

    GetCharacterMovement()->bUseRVOAvoidance = true;
    GetCharacterMovement()->RotationRate.Yaw = 180.f;
    
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

    SetOwnController(NewController);
    //UE_LOG(LogTemp, Display, TEXT("%s PossesedBy :%s"), *(this->GetName()), *(NewController->GetName()));
    _navInvokerComponent->SetActive(true, true);

    // 1. 무브먼트 컴포넌트 확인
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp) return;

    // 2. 컨트롤러로부터 팀 ID 가져오기
    // (컨트롤러가 IGenericTeamAgentInterface를 상속받았다고 가정)
    IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(NewController);
    if (TeamAgent)
    {
        // 컨트롤러의 팀 ID 획득
        uint8 TeamIndex = TeamAgent->GetGenericTeamId().GetId();

        // 3. 비트마스크 변환 (0->1, 1->2 ...)
        int32 MyGroupBit = 1 << TeamIndex;

        // 4. RVO 설정 적용
        MoveComp->SetAvoidanceEnabled(true);
        MoveComp->SetAvoidanceGroup(MyGroupBit);   // 나는 이 팀이다
        MoveComp->SetGroupsToAvoid(MyGroupBit);    // 나는 내 팀만 피한다 (적은 무시)

        // 5. [핵심] 유닛별 고유 가중치 적용
        // 이 코드가 Pawn에 있어야 하는 이유입니다.
        MoveComp->AvoidanceWeight=_unitRVOWeight;

        // (옵션) 자연스러운 이동을 위한 회전 설정
        MoveComp->bOrientRotationToMovement = true;
        bUseControllerRotationYaw = false;
    }


}

void AEnemy::UnPossessed()
{
    Super::UnPossessed();

    _navInvokerComponent->SetActive(false, true);

}

bool AEnemy::SetOwnController(AController* controller)
{
    auto temp = Cast<AEnemyController>(controller);
    if(temp==nullptr)
        return false;
    _controller = temp;
    return true;
}

bool AEnemy::CombineController()
{
    if (_controller!=nullptr)
    {

        _controller->Possess(this);
        return true;
    }
    return false;
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

void AEnemy::SetCombatMovementMode(bool bEnable)
{

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp)
    {
        return;
    }

    if (bEnable)
    {
        MoveBattle();
    }
    else
    {
        MoveNormal();
    }

}

void AEnemy::MoveNormal()
{
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp)
    {
        return;
    }

    // --- 평상시 모드 활성화 (이동 방향 주시) ---
    // 1. 이동 방향으로 회전하는 기능을 다시 켭니다.
    MoveComp->bOrientRotationToMovement = true;

    // 2. 컨트롤러가 회전을 제어할 필요가 없으므로 끕니다.
    MoveComp->bUseControllerDesiredRotation = false;
}

void AEnemy::MoveBattle()
{

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp)
    {
        return;
    }

    // --- 전투 모드 활성화 (타겟 주시) ---
    // 1. 이동 방향으로 강제 회전하는 기능을 끕니다.
    MoveComp->bOrientRotationToMovement = false;

    // 2. 컨트롤러(AI)가 원하는 방향을 바라보도록 허용합니다.
    MoveComp->bUseControllerDesiredRotation = true;
    
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

    bUseControllerRotationYaw = true;
    GetCharacterMovement()->bOrientRotationToMovement = false;
    RaiseAlert();

}

void AEnemy::SetBattleMode()
{

}

void AEnemy::SetNormalMode()
{
}

void AEnemy::TurningBack()
{
    //애니메이션이 마땅치 않다. 애니메이션이 있었다면 애니메이션이 다실행되고나서 호출헀을텐데
    AController* CurrentController = GetController();
    if (CurrentController)
    {
        CurrentController->UnPossess();
    }
    UnitDeactivate();

    _isReadyToSpawn = true;

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
