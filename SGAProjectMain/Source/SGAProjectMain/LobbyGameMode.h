// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
	void BeginPlay();

	void OnMissionSelected(bool hasMission);

	UPROPERTY()
	TArray<class APreDeploymentHellpod*> _hellpods;
};
