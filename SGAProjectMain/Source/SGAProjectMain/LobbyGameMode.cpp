// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "CGameInstance.h"
#include "CSaveGame.h"
#include "Character/PlayerCharacter.h"
#include "Character/HellDiver/HellDiverInvenComponent.h"
#include "Game/PreDeployment/PreDeploymentState.h"
#include "Object/Map/PreDeploymentHellpod.h"
#include "Object/Map/GalacticPlanetGlobe.h"
#include "Data/OperationDataAsset.h"

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

    UCGameInstance* GI = Cast<UCGameInstance>(GetGameInstance());
    if (!GI) return;

	GI->SetGamePhase(EGamePhase::Lobby);

    APlayerCharacter* player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (player)
        player->AddSample(GI->GetSavedSample());

	auto state = GI->GetPreDeployState();
	auto save = GI->GetCurrentSave();

    if (state)
    {
        state->_missionSelectedEvent.AddUObject(this, &ALobbyGameMode::OnMissionSelected);
		state->ApplySaveGameData(save);
    }

  //  if (state->IsOperationCleared())
  //  {
  //      // 성공 UI 띄우기
		//state->ResetOperation();
  //  }
  //  else if (state->IsOperationFailed())
  //  {
  //      // 실패 UI 띄우기
  //      state->ResetOperation();
  //  }

    // 맵에 존재하는 모든 헬포드 캐싱
    for (TActorIterator<APreDeploymentHellpod> It(GetWorld()); It; ++It)
    {
        _hellpods.Add(*It);
    }
}

void ALobbyGameMode::OnMissionSelected(bool hasMission)
{
    for (auto hellpod : _hellpods)
    {
        // 미션이 선택되면 탑승 가능
        hellpod->SetInteractable(hasMission);
	}
}
