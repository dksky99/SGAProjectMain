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

	Init();
	
}

void UBehaviorControlComponent::Init()
{
	_ownerController = Cast<AEnemyController>(GetOwner());
	_ownerPawn = Cast<AEnemy>(_ownerController->GetPawn());


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
	if (type == InType)
		return;
	_ownerPawn->SetUnitState(InType);
	_ownerController->GetBlackboardComponent()->SetValueAsEnum(_unitStateKey, (uint8)InType); // 블랙보드에 키와 값 넣기
}

bool UBehaviorControlComponent::CheckBattleType(EBattleState InType)
{
	return GetCurBattleType() == InType;
}

void UBehaviorControlComponent::ChangeBattleType(EBattleState InType)
{
	EBattleState type = GetCurBattleType(); // 행동전환시 추가 이벤트가 있는 경우 대비
	_ownerController->GetBlackboardComponent()->SetValueAsEnum(_battleStateKey, (uint8)InType); // 블랙보드에 키와 값 넣기
}

EUnitState UBehaviorControlComponent::GetCurUnitType()
{
	return (EUnitState)_ownerController->GetBlackboardComponent()->GetValueAsEnum(_unitStateKey);
}

EBattleState UBehaviorControlComponent::GetCurBattleType()
{
	return (EBattleState)_ownerController->GetBlackboardComponent()->GetValueAsEnum(_battleStateKey);
}


void UBehaviorControlComponent::ChangeDistanceValue(float InValue)
{
	_ownerController->GetBlackboardComponent()->SetValueAsFloat(distanceKey, InValue);
}

float UBehaviorControlComponent::GetDistance()
{
	return _ownerController->GetBlackboardComponent()->GetValueAsFloat(distanceKey);

}

void UBehaviorControlComponent::ChangeAlertThreshold(float InValue)
{
	_ownerController->GetBlackboardComponent()->SetValueAsFloat(alertThresholdKey, InValue);
}

float UBehaviorControlComponent::GetAlertThreshold()
{
	return _ownerController->GetAlertStack();
}

bool UBehaviorControlComponent::GetIsAbleToAct()
{
	return _ownerController->GetBlackboardComponent()->GetValueAsBool(IsAbleToActKey);
}

void UBehaviorControlComponent::SetIsAbleToAct(bool condition)
{
	_ownerController->GetBlackboardComponent()->SetValueAsBool(IsAbleToActKey,condition);
}

bool UBehaviorControlComponent::GetIsTargetLookAtMe()
{
	return _ownerController->GetBlackboardComponent()->GetValueAsBool(IsTargetLookAtMeKey);
}

void UBehaviorControlComponent::SetIsTargetLookAtMe(bool condition)
{

	_ownerController->GetBlackboardComponent()->SetValueAsBool(IsTargetLookAtMeKey, condition);
}


AActor* UBehaviorControlComponent::GetTargetActor()
{
	return _ownerController->GetCurTargetActor();
}

void UBehaviorControlComponent::SetTargetActor(AActor* target)
{
	if (target == nullptr)
		return;
	ITargetable* temp = Cast<ITargetable>(target);
	if(temp)
		_ownerController->GetBlackboardComponent()->SetValueAsObject(targetKey, target);
}

FVector UBehaviorControlComponent::GetTargetLoc()
{
	return _ownerController->GetTargetLoc();

}

void UBehaviorControlComponent::SetTargetLoc(FVector loc)
{
	_ownerController->GetBlackboardComponent()->SetValueAsVector(targetLocKey, loc);
}
