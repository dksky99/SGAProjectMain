// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "Kismet/GameplayStatics.h"
#include "CGameInstance.h"
#include "Character/PlayerCharacter.h"
#include "Character/HellDiver/HellDiverInvenComponent.h"

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

    UCGameInstance* GI = Cast<UCGameInstance>(GetGameInstance());
    if (!GI) return;

    APlayerCharacter* player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (player)
    {
        player->AddSample(GI->GetSavedSample());
    }
}
