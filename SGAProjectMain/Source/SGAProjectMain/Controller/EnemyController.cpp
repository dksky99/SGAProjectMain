// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyController.h"

#include "NavigationSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AIPerceptionTypes.h"  // FAIStimulus\

#include "GenericTeamAgentInterface.h" // 이 헤더파일을 cpp 상단에 include 해야 합니다.
#include "Blueprint/AIBlueprintHelperLibrary.h"

#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "../Character/Enemy/Enemy.h"
#include "../Character/StatComponent.h"
#include "../Character/Enemy/BehaviorControlComponent.h"

#include "Algo/MinElement.h"
AEnemyController::AEnemyController()
{

    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

    _sightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));

    _sightConfig->SightRadius = 1500.f  ;
    _sightConfig->LoseSightRadius = 2000.f;
    _sightConfig->PeripheralVisionAngleDegrees = 160.f;

    //_sightConfig->SetMaxAge(5.f);
    //_sightConfig->AutoSuccessRangeFromLastSeenLocation = -1.f;

    _sightConfig->DetectionByAffiliation.bDetectEnemies = true;
    _sightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    _sightConfig->DetectionByAffiliation.bDetectNeutrals = false;

    PerceptionComponent->ConfigureSense(*_sightConfig);
    PerceptionComponent->SetDominantSense(_sightConfig->GetSenseImplementation());


    _hearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing Config"));
    _hearingConfig->HearingRange = 700.f;
    _hearingConfig->SetMaxAge(3.f);

    _hearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    _hearingConfig->DetectionByAffiliation.bDetectNeutrals = false;
    _hearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComponent->ConfigureSense(*_hearingConfig);


    _damageSenseConfig= CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("Damage Config"));
    
    PerceptionComponent->ConfigureSense(*_damageSenseConfig);


    TeamId = FGenericTeamId((uint8)(ETeamID::Enemy));

    PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyController::OnTargetDetected);
    PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &AEnemyController::PerceptionUpdated);
}

void AEnemyController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    AEnemy* pawnTemp = Cast<AEnemy>(InPawn);
    if (pawnTemp)
    {
        _pawn = pawnTemp;
        _blackBoard = pawnTemp->GetBlackboardData();
        _behaviorTree = pawnTemp->GetBehaviorTree();
        _behaviorControlComponent = pawnTemp->GetBehaviorControl();
        _behaviorControlComponent->Init();
        SetAlertStep(_pawn->GetUnitState());
    }
    if (PerceptionComponent)
    {
        PerceptionComponent->Activate(true);
    }
    UBlackboardComponent* temp = Blackboard;
    IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(InPawn);
    if (TeamAgent)
    {
        TeamAgent->SetGenericTeamId(TeamId);
    }
    if (UseBlackboard(_blackBoard, temp))
    {

        if (RunBehaviorTree(_behaviorTree))
        {

            
        }
        else
        {

            UE_LOG(LogTemp, Error, TEXT("%s :  BT_Failed"),*(InPawn->GetName()));
        }



    }
    else
    {

        UE_LOG(LogTemp, Error, TEXT("%s :  BB_Failed"), *(InPawn->GetName()));
    }
}

void AEnemyController::OnUnPossess()
{
    Super::OnUnPossess();

    UBrainComponent* Brain = GetBrainComponent();
    if (Brain)
    {
        Brain->StopLogic(TEXT("Unpossessed"));
    }

    // <<< 2. 퍼셉션 컴포넌트 비활성화 >>>
    // 이 함수가 호출되면 퍼셉션 컴포넌트가 더 이상 틱을 돌지 않고, 감각 업데이트를 수신하지 않습니다.
    if (PerceptionComponent)
    {
        PerceptionComponent->Deactivate();
    }

    _pawn = nullptr;
    _blackBoard = nullptr;
    _behaviorTree = nullptr;
    ResetController();
}

void AEnemyController::BeginPlay()
{
    Super::BeginPlay();
    _curTarget = nullptr;

}

void AEnemyController::ResetController()
{

    _isReadyToStack = true;
     _targets.Empty();


     _curTarget = nullptr;

    _alertStep = EAIAlertStep::None;


   _curLoudnessStack = 0.0f;
}

void AEnemyController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{

    if (Stimulus.WasSuccessfullySensed())
    {
        UE_LOG(LogTemp, Warning, TEXT("적 감지: %s"), *Actor->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("적 놓침: %s"), *Actor->GetName());
    }

}

void AEnemyController::PerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* UpdatedActor : UpdatedActors)
    {
        
        FAIStimulus AIStimulus;
        AIStimulus = CanSenseActor(UpdatedActor, EAIPerceptionSense::EPS_Sight);
        if (AIStimulus.WasSuccessfullySensed())
        {
            HandleSensedSight(UpdatedActor);
        }
        AIStimulus = CanSenseActor(UpdatedActor, EAIPerceptionSense::EPS_Hearing);
        if (AIStimulus.WasSuccessfullySensed())
        {
            HandleSensedHearing(AIStimulus.StimulusLocation);
        }
        AIStimulus = CanSenseActor(UpdatedActor, EAIPerceptionSense::EPS_Damage);
        if (AIStimulus.WasSuccessfullySensed())
        {
            HandleSensedDamage(UpdatedActor);
        }
    }

}

void AEnemyController::HandleSensedSight(AActor* Actor)
{

    if (_pawn == nullptr)
        return;
    if (CheckTargetable(Actor) == false)
        return;

    ITargetable* temp = Cast<ITargetable>(Actor);
    if (temp == nullptr)
        return;

    _curTarget = Actor;
    UE_LOG(LogTemp, Display, TEXT("Hostile Target Acquired by Sight: %s"), *Actor->GetName());
   
}
void AEnemyController::HandleSensedHearing(FVector directionHeared)
{

    if (_pawn == nullptr)
        return;

    _lastSensedLoc = directionHeared;
    if (_isReadyToStack == false)
        return;
    _isReadyToStack = false;
    GetWorld()->GetTimerManager().SetTimer(
        _alertStackTimer, // 관리할 타이머 핸들
        this,                      // 타이머가 만료됐을 때 함수를 호출할 오브젝트
        &AEnemyController::ReadyToStack, // 호출될 함수
        _alertStackTime,                     // 지연 시간 (초)
        false                      // 반복 여부 (false = 한 번만 실행)
    );
    
	 _curLoudnessStack +=1.f;
    

     if (_pawn->GetUnitState() == EUnitState::InBattle)
     {
         return;
     }
     if (GetWorld()->GetTimerManager().IsTimerActive(_alertResetTimer))
     {
         // 이미 실행 중이라면, 기존 타이머를 중단하고 새로 시작
         GetWorld()->GetTimerManager().ClearTimer(_alertResetTimer);
     }
     GetWorld()->GetTimerManager().SetTimer(
         _alertResetTimer, // 관리할 타이머 핸들
         this,                      // 타이머가 만료됐을 때 함수를 호출할 오브젝트
         &AEnemyController::ResetAlertStack, // 호출될 함수
         _alertResetTime,                     // 지연 시간 (초)
         false                      // 반복 여부 (false = 한 번만 실행)
     );
}

void AEnemyController::HandleSensedDamage(AActor* Actor)
{

    if (_pawn == nullptr)
        return;
    if (CheckTargetable(Actor) == false)
        return;

    ITargetable* temp = Cast<ITargetable>(Actor);
    if (temp == nullptr)
        return;

    _curTarget = Actor;

}

FAIStimulus AEnemyController::CanSenseActor(AActor* Actor, EAIPerceptionSense AIPerceptionSense)
{
    FActorPerceptionBlueprintInfo ActorPerceptionBlueprintInfo;
    FAIStimulus ResultStimulus;

    PerceptionComponent->GetActorsPerception(Actor, ActorPerceptionBlueprintInfo);

    TSubclassOf<UAISense> QuerySenseClass;
    switch (AIPerceptionSense)
    {
    case EAIPerceptionSense::EPS_None:
        break;
    case EAIPerceptionSense::EPS_Sight:
        QuerySenseClass = UAISense_Sight::StaticClass();
        break;
    case EAIPerceptionSense::EPS_Hearing:
        QuerySenseClass = UAISense_Hearing::StaticClass();
        break;
    case EAIPerceptionSense::EPS_Damage:
        QuerySenseClass = UAISense_Damage::StaticClass();
        break;
    case EAIPerceptionSense::EPS_MAX:
        break;
    default:
        break;
    }

    TSubclassOf<UAISense> LastSensedStimulusClass;

    for (const FAIStimulus& AIStimulus : ActorPerceptionBlueprintInfo.LastSensedStimuli)
    {
        LastSensedStimulusClass = UAIPerceptionSystem::GetSenseClassForStimulus(this, AIStimulus);


        if (QuerySenseClass == LastSensedStimulusClass)
        {
            ResultStimulus = AIStimulus;
            return ResultStimulus;
        }

    }
    return ResultStimulus;
}

void AEnemyController::SetAlertStep(EUnitState unitState)
{
    switch (unitState)
    {
    case EUnitState::Stay:
    case EUnitState::Patrol:
        SetLowAlert();
        break;
    case EUnitState::Weak_Alert:
        SetMediumAlert();
        break;
    case EUnitState::Strong_Alert:
    case EUnitState::InBattle:
        SetHighAlert();
        break;
    case EUnitState::MAX:
        break;
    default:
        break;
    }
}

void AEnemyController::SetNone()
{
    if (_alertStep == EAIAlertStep::None)
        return;
    _alertStep = EAIAlertStep::None;
    _hearingConfig->HearingRange = 0.f;
    _sightConfig->SightRadius = 0.f;
}

void AEnemyController::SetLowAlert()
{
    if (_alertStep == EAIAlertStep::LowAlert)
        return;
    _alertStep = EAIAlertStep::LowAlert;

    _hearingConfig->HearingRange = 500.f;
    _sightConfig->SightRadius = 1000.f;
}

void AEnemyController::SetMediumAlert()
{
    if (_alertStep == EAIAlertStep::MediumAlert)
        return;
    _alertStep = EAIAlertStep::MediumAlert;
    _hearingConfig->HearingRange = 700.f;
    _sightConfig->SightRadius = 1500.f;
}

void AEnemyController::SetHighAlert()
{
    if (_alertStep == EAIAlertStep::HighAlert)
        return;
    _alertStep = EAIAlertStep::HighAlert;
    _hearingConfig->HearingRange = 1000.f;
    _sightConfig->SightRadius = 2000.f;
}

void AEnemyController::AddTargetActor(AActor* target)
{
    _targets.AddUnique(target);

    if (_curTarget == nullptr)
        _curTarget = target;

}

void AEnemyController::AddAlertStack(float loudness)
{

    _curLoudnessStack += loudness;
}

void AEnemyController::ReadyToStack()
{
    _isReadyToStack = true;
}

void AEnemyController::ResetAlertStack()
{

    _curLoudnessStack = 0.0f;
}

AActor* AEnemyController::GetCurTargetActor()
{
    if (CheckTargetRefresh())
    {
        //RefreshTargets();
        _curTarget = nullptr;
    }
    return _curTarget;
}

AActor* AEnemyController::GetNewTargetActor()
{
    RefreshTargets();
    if(_targets.IsEmpty())
        return nullptr;
    AActor* pawn = GetPawn();
    auto min = Algo::MinElementBy(_targets, [pawn](AActor* target) {return target->GetDistanceTo(pawn); });
    
    return *min;

    
}

bool AEnemyController::CheckTargetRefresh()
{
    ITargetable* targetable = Cast<ITargetable>(_curTarget);
    if (targetable == nullptr)
        return true;
    if (targetable->IsTargetable() == false)
        return true;
    if (_curTarget->IsHidden())
        return true;
    return false;
}

void AEnemyController::RefreshTargets()
{
    if (_curTarget == nullptr)
        return;
    ITargetable* targetable = Cast<ITargetable>(_curTarget);
    if (targetable == nullptr)
    {
        _curTarget = nullptr;
        return;
    }
    if (targetable->IsTargetable() == false)
    {
        _curTarget = nullptr;
        return;

    }

}

void AEnemyController::RecieveTarget(AActor* target)
{
    auto temp = Cast<ITargetable>(target);
    if (temp)
    {

        _curTarget = target;
    }
}

void AEnemyController::RecieveTargetLoc(FVector targetLoc)
{
    _curLoudnessStack = 6.0f;
    _lastSensedLoc = targetLoc;
}

ETeamAttitude::Type AEnemyController::GetTeamAttitudeTowards(const AActor& Other) const
{
    const APawn* OtherPawn = Cast<const APawn>(&Other);
    if (OtherPawn == nullptr)
    {
        return ETeamAttitude::Neutral;
    }

    // 2. 상대방이 나와 같은 'IGenericTeamAgentInterface'를 상속받은 AI(Enemy)인지 확인
    auto OtherTeamAgent = Cast<const IGenericTeamAgentInterface>(&Other);
    if (OtherTeamAgent && OtherTeamAgent->GetGenericTeamId() == TeamId)
    {
        // 나와 팀 ID가 같다면 '아군(Friendly)' -> 퍼셉션 필터에서 걸러짐
        return ETeamAttitude::Friendly;
    }

    // 3. 상대방이 플레이어(ITargetable 인터페이스를 가진 대상)라면 '적대적(Hostile)'
    // 주의: 플레이어 캐릭터 클래스가 ITargetable을 상속받고 있어야 합니다.
    auto Targetable = Cast<const ITargetable>(&Other);
    if (Targetable && Targetable->IsTargetable()) // 필요하다면 IsTargetable 체크
    {
        // 플레이어는 적으로 간주 -> 퍼셉션이 감지함
        return ETeamAttitude::Hostile;
    }

    // 그 외는 중립
    return ETeamAttitude::Neutral;
}

bool AEnemyController::CheckTargetable(AActor* target)
{

    if (target == nullptr || target == GetPawn()) // 자기 자신은 제외
        return false;
    IGenericTeamAgentInterface* TargetTeamAgent = Cast<IGenericTeamAgentInterface>(target);
    if (TargetTeamAgent == nullptr) // 팀 구분이 없는 대상은 무시
        return false;
    // GetTeamAttitudeTowards 함수로 나와 상대방의 관계를 확인합니다.
    ETeamAttitude::Type Attitude = GetTeamAttitudeTowards(*target);
    // 관계가 '적대적(Hostile)'일 경우에만 타겟으로 설정합니다.
    if (Attitude != ETeamAttitude::Hostile)
        return false;

    AEnemy* enemy = Cast<AEnemy>(target);
    if (enemy)
        return false;
    //타겟어블 계승한 클래스가아니면 false
    ITargetable* targetable = Cast<ITargetable>(target);
    if (targetable == nullptr)
        return false;
    //타겟어블은 타겟팅이 될수있는상태인지를 추상함수를 갖음. 그 조건에 안맞으면 false
    if (targetable->IsTargetable() == false)
        return false;
    //아군 AI유닛이 없는 이상황엔 Enemy를 계승하면 전부 같은 편 타겟에서 제외
    return true;
}
