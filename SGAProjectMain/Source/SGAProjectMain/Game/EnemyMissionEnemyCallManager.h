// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyMissionEnemyCallManager.generated.h"
/*
	여기서 주둔지의 팩토리와 스쿼드를 연결해줄 예정이다.



*/
UCLASS()
class SGAPROJECTMAIN_API AEnemyMissionEnemyCallManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemyMissionEnemyCallManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:





};
