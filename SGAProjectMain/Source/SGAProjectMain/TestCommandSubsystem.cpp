// Fill out your copyright notice in the Description page of Project Settings.


#include "TestCommandSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Character/CharacterBase.h"
#include "MainGameMode.h"

void UTestCommandSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    KillMeDelegate = FConsoleCommandDelegate::CreateUObject(this, &UTestCommandSubsystem::OnKillMe);

    KillMeCommand = IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("KillMe"),
        TEXT("자기 캐릭터를 죽입니다."),
        KillMeDelegate,
        ECVF_Cheat
    );


    KnockDownDelegate = FConsoleCommandDelegate::CreateUObject(this, &UTestCommandSubsystem::OnSelfKnockDown);
    KnockDownCommand = IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("SelfKnockDown"),
        TEXT("자기 자신을 녹다운 상태로 만듭니다."),
        KnockDownDelegate,
        ECVF_Cheat
    );


    MoveLobbyDelegate = FConsoleCommandDelegate::CreateUObject(this, &UTestCommandSubsystem::OnMoveLobby);
    MoveLobbyCommand = IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("MoveLevel"),
        TEXT("게임을 종료하고 로비로 이동합니다."),
        MoveLobbyDelegate,
        ECVF_Cheat
    );
}

void UTestCommandSubsystem::Deinitialize()
{
    if (KillMeCommand)
    {
        IConsoleManager::Get().UnregisterConsoleObject(KillMeCommand);
        KillMeCommand = nullptr;
    }

    if (KnockDownCommand)
    {
        IConsoleManager::Get().UnregisterConsoleObject(KnockDownCommand);
        KnockDownCommand = nullptr;
    }

    if (MoveLobbyCommand)
    {
        IConsoleManager::Get().UnregisterConsoleObject(MoveLobbyCommand);
        MoveLobbyCommand = nullptr;
    }
    Super::Deinitialize();
}

void UTestCommandSubsystem::OnKillMe()
{
    UWorld* World = GEngine->GetWorldFromContextObjectChecked(this);
    if (!World) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC) return;

    ACharacterBase* MyChar = Cast<ACharacterBase>(PC->GetPawn());
    if (MyChar)
    {
        UGameplayStatics::ApplyDamage(MyChar, 2000, nullptr, nullptr, nullptr);
    }
}

void UTestCommandSubsystem::OnSelfKnockDown()
{
    UWorld* World = GEngine->GetWorldFromContextObjectChecked(this);
    if (!World) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC) return;

    ACharacterBase* MyChar = Cast<ACharacterBase>(PC->GetPawn());
    if (MyChar)
    {
        MyChar->KnockDown();
    }
}

void UTestCommandSubsystem::OnMoveLobby()
{
    UWorld* World = GEngine->GetWorldFromContextObjectChecked(this);
    if (!World) return;

    AMainGameMode* GM = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this));
    if (GM)
    {
        GM->OnBattleEnd();
    }
}
