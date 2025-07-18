// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyController.h"

#include "NavigationSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AIPerceptionTypes.h"  // FAIStimulus


AEnemyController::AEnemyController()
{

    _aIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

    _sightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));

    _sightConfig->SightRadius = 1000.f;
    _sightConfig->LoseSightRadius = 1200.f;
    _sightConfig->PeripheralVisionAngleDegrees = 60.f;

    _sightConfig->DetectionByAffiliation.bDetectEnemies = true;
    _sightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    _sightConfig->DetectionByAffiliation.bDetectNeutrals = true;

    _aIPerception->ConfigureSense(*_sightConfig);
    _aIPerception->SetDominantSense(_sightConfig->GetSenseImplementation());
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
    _aIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyController::OnTargetDetected);

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
