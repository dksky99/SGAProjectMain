// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Containers/Queue.h"
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

	void InitSquad();
public:	
	// Called every frame

	void ReinforceHelldiver(FVector callPoint);

	void ReturnDeadController(AController* controller);



protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class APlayerCharacter> _hellDiverClass;

	UPROPERTY()
	TArray<class APlayerCharacter*> _hellDiverPool;


	FTimerHandle _reinforceBudgetTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	float _addReinforceBudgetCoolTime = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	int _remainReinforceBudget = 5;
	
	TQueue<AController*> _controllerQ;
	

};
