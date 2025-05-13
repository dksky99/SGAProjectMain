// Fill out your copyright notice in the Description page of Project Settings.


#include "HellDiver.h"
#include "GameFramework/Character.h" // 이게 필요함
#include "HellDiverMovementComponent.h"
#include "HellDiverStateComponent.h"
#include "HellDiverStatComponent.h"



AHellDiver::AHellDiver(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UHellDiverMovementComponent>(ACharacter::CharacterMovementComponentName))
{
    GetCharacterMovement()->JumpZVelocity = 300.0f;

    _stateComponent = CreateDefaultSubobject<UHellDiverStateComponent>("State");


    _statComponent = CreateDefaultSubobject<UHellDiverStatComponent>("Stat");
}

UHellDiverStateComponent* AHellDiver::GetStateComponent()
{
    return _stateComponent;
}

void AHellDiver::StartSprint()
{
    if (_stateComponent->StartSprint() == false)
        return;

    auto movement=GetMovementComponent();
    GetCharacterMovement()->MaxWalkSpeed = _statComponent->GetSprintSpeed();
}

void AHellDiver::FinishSprint()
{
    if (_stateComponent->FinishSprint() == false)
        return;

    auto movement = GetMovementComponent();
    GetCharacterMovement()->MaxWalkSpeed = _statComponent->GetDefaultSpeed();
}

void AHellDiver::StartCrouch()
{
    if (_stateComponent->StartCrouch() == false)
        return;

    auto movement = GetMovementComponent();
    GetCharacterMovement()->MaxWalkSpeed = _statComponent->GetCrouchSpeed();
}

void AHellDiver::FinishCrouch()
{
    if (_stateComponent->FinishCrouch() == false)
        return;

    auto movement = GetMovementComponent();
    GetCharacterMovement()->MaxWalkSpeed = _statComponent->GetDefaultSpeed();
}

void AHellDiver::StartProne()
{
    if (_stateComponent->StartProne() == false)
        return;

    auto movement = GetMovementComponent();
    GetCharacterMovement()->MaxWalkSpeed = _statComponent->GetProneSpeed();
}

void AHellDiver::FinishProne()
{
    if (_stateComponent->FinishProne() == false)
        return;
    auto movement = GetMovementComponent();
    GetCharacterMovement()->MaxWalkSpeed = _statComponent->GetDefaultSpeed();
}

void AHellDiver::Rolling()
{
    if (this->CanJump() == false)
        return;
    if (_stateComponent->IsRolling())
        return;
    _stateComponent->StartRolling();
    Jump();

    FVector forward = GetActorForwardVector();

    float forwardBoost = 1000.0f; 
    FVector boost = forward * forwardBoost;

    // 4. 현재 Velocity에 더해줌
    GetCharacterMovement()->Velocity += boost;

    StartProne();
}

void AHellDiver::FinishRolling()
{

    _stateComponent->FinishRolling();
}

void AHellDiver::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);

    if (_stateComponent->IsRolling())
    {
        // 일정 시간 후 복구
        GetWorld()->GetTimerManager().SetTimer(
            _rollingTimerHandle, this, &AHellDiver::FinishRolling, 0.2, false
        );

    }


}

