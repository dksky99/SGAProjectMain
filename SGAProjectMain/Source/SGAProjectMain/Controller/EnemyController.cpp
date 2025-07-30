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

#include "Blueprint/AIBlueprintHelperLibrary.h"

#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "../Character/Enemy/Enemy.h"


AEnemyController::AEnemyController()
{

    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

    _sightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));

    _sightConfig->SightRadius = 1000.f;
    _sightConfig->LoseSightRadius = 1200.f;
    _sightConfig->PeripheralVisionAngleDegrees = 60.f;

    //_sightConfig->SetMaxAge(5.f);
    //_sightConfig->AutoSuccessRangeFromLastSeenLocation = -1.f;

    _sightConfig->DetectionByAffiliation.bDetectEnemies = true;
    _sightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    _sightConfig->DetectionByAffiliation.bDetectNeutrals = true;

    PerceptionComponent->ConfigureSense(*_sightConfig);
    PerceptionComponent->SetDominantSense(_sightConfig->GetSenseImplementation());


    _hearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing Config"));
    _hearingConfig->HearingRange = 500.f;
    _hearingConfig->SetMaxAge(3.f);

    _hearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    _hearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    _hearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    PerceptionComponent->ConfigureSense(*_hearingConfig);


    _damageSenseConfig= CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("Damage Config"));
    
    PerceptionComponent->ConfigureSense(*_damageSenseConfig);


}

void AEnemyController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    UBlackboardComponent* temp = Blackboard;

    if (UseBlackboard(_blackBoard, temp))
    {

        if (RunBehaviorTree(_behaviorTree))
        {

            UE_LOG(LogTemp, Error, TEXT("BT_Success"));

            UE_LOG(LogTemp, Display, TEXT("%s Use BehaviourTree :%s"), *(this->GetName()), *(_behaviorTree->GetName()));
            UE_LOG(LogTemp, Display, TEXT("%s Use BlackBoard :%s"), *(this->GetName()), *(_blackBoard->GetName()));
            
        }
        else
        {

            UE_LOG(LogTemp, Error, TEXT("BT_Failed"));
        }



    }
    else
    {

        UE_LOG(LogTemp, Error, TEXT("BB_Failed"));
    }
}

void AEnemyController::OnUnPossess()
{
    Super::OnUnPossess();
}

void AEnemyController::BeginPlay()
{
    Super::BeginPlay();
    PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyController::OnTargetDetected);
    PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &AEnemyController::PerceptionUpdated);
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
}

void AEnemyController::HandleSensedHearing(FVector directionHeared)
{
}

void AEnemyController::HandleSensedDamage(AActor* Actor)
{
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
