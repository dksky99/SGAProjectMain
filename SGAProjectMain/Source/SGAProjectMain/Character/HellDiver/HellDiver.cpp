// Fill out your copyright notice in the Description page of Project Settings.


#include "HellDiver.h"
#include "GameFramework/Character.h" // 이게 필요함
#include "HellDiverMovementComponent.h"
#include "HellDiverStateComponent.h"



AHellDiver::AHellDiver(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UHellDiverMovementComponent>(ACharacter::CharacterMovementComponentName))
{

    _stateComponent = CreateDefaultSubobject<UHellDiverStateComponent>("State");


}

UHellDiverStateComponent* AHellDiver::GetStateComponent()
{
    return _stateComponent;
}
