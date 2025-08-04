// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyReinforceManager.generated.h"

UCLASS()
class SGAPROJECTMAIN_API AEnemyReinforceManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemyReinforceManager();



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Init();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	TMap< TSubclassOf<class AReinforcementSquad>, int >_squadList;



	UPROPERTY()
	TArray<class AReinforcementSquad*> _squadPool;

};
