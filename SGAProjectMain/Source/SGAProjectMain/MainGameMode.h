// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainGameMode.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AMainGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	void CallEscapePlane();

	void OnBattleEnd();
	
private:
	UPROPERTY(EditAnywhere, Category = "Game/Plane")
	TSubclassOf<class AEscapePlane> _escapePlaneClass;

	UPROPERTY(EditAnywhere, Category = "Game/Plane")
	FVector _planeSpawnLoc;
};
