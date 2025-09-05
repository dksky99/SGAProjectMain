// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorControlComponent.h"
#include "Enemy.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../../Controller/EnemyController.h"
#include "Enemy.h"
// Sets default values for this component's properties
UBehaviorControlComponent::UBehaviorControlComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UBehaviorControlComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	_ownerController = Cast<AEnemyController>(GetOwner());
	_ownerPawn = Cast<AEnemy>(_ownerController->GetPawn());
	blackboard = _ownerController->GetBlackboardComponent();
	
}


// Called every frame
void UBehaviorControlComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UBehaviorControlComponent::CheckUnitType(EUnitState InType)
{
	return  GetCurUnitType() == InType;
}

void UBehaviorControlComponent::ChangeUnitType(EUnitState InType)
{

	EUnitState type = GetCurUnitType(); // 행동전환시 추가 이벤트가 있는 경우 대비
	_ownerPawn->SetUnitState(InType);
	blackboard->SetValueAsEnum(_unitStateKey, (uint8)InType); // 블랙보드에 키와 값 넣기
}

bool UBehaviorControlComponent::CheckBattleType(EBattleState InType)
{
	return GetCurBattleType() == InType;
}

void UBehaviorControlComponent::ChangeBattleType(EBattleState InType)
{
	EBattleState type = GetCurBattleType(); // 행동전환시 추가 이벤트가 있는 경우 대비
	blackboard->SetValueAsEnum(_battleStateKey, (uint8)InType); // 블랙보드에 키와 값 넣기
}

EUnitState UBehaviorControlComponent::GetCurUnitType()
{
	return (EUnitState)blackboard->GetValueAsEnum(_unitStateKey);
}

EBattleState UBehaviorControlComponent::GetCurBattleType()
{
	return (EBattleState)blackboard->GetValueAsEnum(_battleStateKey);
}


void UBehaviorControlComponent::ChangeDistanceValue(float InValue)
{
	blackboard->SetValueAsFloat(distanceKey, InValue);
}

float UBehaviorControlComponent::GetDistance()
{
	return blackboard->GetValueAsFloat(distanceKey);

}

void UBehaviorControlComponent::ChangeAlertThreshold(float InValue)
{
	blackboard->SetValueAsFloat(alertThresholdKey, InValue);
}

float UBehaviorControlComponent::GetAlertThreshold()
{
	return _ownerController->GetAlertStack();
}


AActor* UBehaviorControlComponent::GetTargetActor()
{
	return _ownerController->GetCurTargetActor();
}

void UBehaviorControlComponent::SetTargetActor(AActor* target)
{
	ITargetable* temp = Cast<ITargetable>(target);
	if(temp)
	blackboard->SetValueAsObject(targetKey, target);
}

FVector UBehaviorControlComponent::GetTargetLoc()
{
	return _ownerController->GetTargetLoc();

}

void UBehaviorControlComponent::SetTargetLoc(FVector loc)
{
	blackboard->SetValueAsVector(targetLocKey, loc);
}
