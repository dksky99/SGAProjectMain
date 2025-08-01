// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../EnemySquad.h"
#include "StationedSquad.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AStationedSquad : public AEnemySquad
{
	GENERATED_BODY()
	
public:
	AStationedSquad();

	virtual void BeginPlay() override;

	void CallRemainUnit();


protected:

	FTimerHandle _GenerateTimer;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	float _generateCoolDown = 5.0f;

	bool _isGeneratable = true;

	bool _isActivating = true;



};
