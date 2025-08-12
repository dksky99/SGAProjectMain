// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TestCommandSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UTestCommandSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:
    FConsoleCommandDelegate KillMeDelegate;
    FConsoleCommandDelegate KnockDownDelegate;
    FConsoleCommandDelegate MoveLobbyDelegate;
    FConsoleCommandDelegate MoveMainGameDelegate;
    FConsoleCommandDelegate CallEscapePlaneDelegate;
    FConsoleCommandDelegate CallEnemyReinforceDelegate;

    IConsoleObject* KillMeCommand;
    IConsoleObject* KnockDownCommand;
    IConsoleObject* MoveLobbyCommand;
    IConsoleObject* MoveMainGameCommand;
    IConsoleObject* CallEscapePlaneCommand;
    IConsoleObject* CallEnemyReinforceCommand;

    void OnKillMe();
    void OnSelfKnockDown();
    void OnMoveLobby();
    void OnMoveMainGame();
    void OnCallEscapePlane();
    void OnCallEnemyReinforce();
};
