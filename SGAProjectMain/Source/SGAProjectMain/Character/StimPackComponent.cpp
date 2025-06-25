// Fill out your copyright notice in the Description page of Project Settings.


#include "StimPackComponent.h"
#include "HellDiver/HellDiver.h"
#include "HellDiver/HellDiverStatComponent.h"

// Sets default values for this component's properties
UStimPackComponent::UStimPackComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UStimPackComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	_owner = Cast<AHellDiver>(GetOwner());
}


// Called every frame
void UStimPackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UStimPackComponent::UseStimPack()
{
	if (_curStimPack <= 0)
		return;

	_curStimPack--;

	StartOrExtendRegen();

	if (_stimPackChanged.IsBound())
		_stimPackChanged.Broadcast(_curStimPack, _maxStimPack);
}

void UStimPackComponent::RefillStimPack()
{
	_curStimPack += 2;

	if (_curStimPack > _maxStimPack)
		_curStimPack = _maxStimPack;

	if (_stimPackChanged.IsBound())
		_stimPackChanged.Broadcast(_curStimPack, _maxStimPack);
}

void UStimPackComponent::HandleRegen()
{
	if (_ticksElapsed++ >= _maxTicks)
	{
		GetWorld()->GetTimerManager().ClearTimer(_regenTimerHandle);
		return;
	}


	if (auto StatComp = _owner->GetStatComponent())
	{
		StatComp->ChangeHp(AmountPerTick);
	}
}

void UStimPackComponent::StartOrExtendRegen()
{
	int32 NewTicks = FMath::CeilToInt(TotalRegenDuration / RegenInterval);

	if (GetWorld()->GetTimerManager().IsTimerActive(_regenTimerHandle))
	{
		// 이미 회복 중이면 남은 틱에 누적
		_maxTicks += NewTicks;
	}
	else
	{
		// 최초 시작
		_ticksElapsed = 0;
		_maxTicks = NewTicks;

		GetWorld()->GetTimerManager().SetTimer(
			_regenTimerHandle,
			this, &UStimPackComponent::HandleRegen,
			RegenInterval,
			true
		);
	}
}

void UStimPackComponent::BroadcastStimPackChanged()
{
	if (_stimPackChanged.IsBound())
		_stimPackChanged.Broadcast(_curStimPack, _maxStimPack);
}

