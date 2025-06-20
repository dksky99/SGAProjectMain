// Fill out your copyright notice in the Description page of Project Settings.


#include "StatComponent.h"

// Sets default values for this component's properties
UStatComponent::UStatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UStatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UStatComponent::ChangeHp(float amount)
{
	_curHp += amount;

	if (_curHp > _maxHp)
		_curHp = _maxHp;

	IsDead();
}

bool UStatComponent::IsDead()
{
	if (_curHp <= 0)
	{
		Dead();
		return true;
	}

	return false;
}

void UStatComponent::Dead()
{

}

