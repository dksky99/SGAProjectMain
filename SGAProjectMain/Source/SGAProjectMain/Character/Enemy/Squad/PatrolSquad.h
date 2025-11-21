// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../EnemySquad.h"
#include "PatrolSquad.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API APatrolSquad : public AEnemySquad
{
	GENERATED_BODY()
public:




protected:

	TArray<class AEnemy*> _units;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Patrol", meta = (AllowPrivateAccess = "true"))
	class ACPatrolPath* _paths;

	
};
