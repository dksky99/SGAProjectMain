// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemSpawner.generated.h"

USTRUCT(BlueprintType)
struct FSpawnList
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Game/Item")
	TSubclassOf<AActor> _spawnClass; // 소환할 클래스

	UPROPERTY(EditAnywhere, Category = "Game/Item")
	int32 _count = 1; // 소환 개수
};

UCLASS()
class SGAPROJECTMAIN_API AItemSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	void SpawnItem();

	FVector FindSpawnLocation();


	UPROPERTY(EditAnywhere, Category = "Game/Item")
	TArray<FSpawnList> _spawnList;
};
