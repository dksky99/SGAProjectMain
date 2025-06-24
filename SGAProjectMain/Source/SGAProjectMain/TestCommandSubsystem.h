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

    IConsoleObject* KillMeCommand;
    IConsoleObject* KnockDownCommand;

    void OnKillMe();
    void OnSelfKnockDown();
};
