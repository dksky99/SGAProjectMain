// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyPatrolManager.generated.h"

UCLASS()
class SGAPROJECTMAIN_API AEnemyPatrolManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemyPatrolManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Init();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	class APatrolSquad* GetExtraCallableSquad(FVector callPoint, AActor* target = nullptr);

	bool DeploySquad(class APatrolSquad* squad, class ACPatrolPath* path);
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	TMap< TSubclassOf<class APatrolSquad>, int32 >_squadList;



	UPROPERTY()
	TArray<class APatrolSquad*> _squadPool;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	TArray<class ACPatrolPath*> _paths;

	


};
