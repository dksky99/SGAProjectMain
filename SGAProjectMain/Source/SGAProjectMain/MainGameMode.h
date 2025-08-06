// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "MainGameMode.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AMainGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;

	virtual void StartPlay() override;

	void CallEscapePlane();

	void OnBattleEnd();
	
private:
	UPROPERTY(EditAnywhere, Category = "Game/Plane")
	TSubclassOf<class AEscapePlane> _escapePlaneClass;

	UPROPERTY(EditAnywhere, Category = "Game/Plane")
	FVector _planeSpawnLoc;


	UPROPERTY(EditAnywhere, Category = "Game/GamePlay")
	TSubclassOf<class AEnemyReinforceManager> _enemyReinforceManagerClass;

	UPROPERTY(VisibleAnywhere, Category = "Game/EnemyReinforce")
	class AEnemyReinforceManager* _enemyReinforceManager;

};
