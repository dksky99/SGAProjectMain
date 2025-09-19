// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HelldiverReinforceManager.generated.h"

UCLASS()
class SGAPROJECTMAIN_API AHelldiverReinforceManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHelldiverReinforceManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame


protected:

	FTimerHandle _reinforceBudgetTimer;

	float _addReinforceBudgetCoolTime = 120.f;

	uint32 _remainReinforceBudget = 5;


};
