// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyPatrolManager.generated.h"

/*
	패트롤 매니저는 게임이 시작되면 4개의 스쿼드를 각자 4개의 패트로패스에 분배하여 반복하여 순찰을 돌게하는 매니저다.
	스쿼드들은 전멸하고 다시 증원이 가능해지면 다시 시작점에서 호출된다.



*/


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

	
	void GetExtraCallableSquad();

	bool DeploySquad(class AEnemySquad* squad, class AReinforceSquadSpawner* spawner, class ACPatrolPath* path);
	
	bool CheckRestoredSquad(class AEnemySquad* squad, class AReinforceSquadSpawner* spawner );

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	TMap< TSubclassOf<class AEnemySquad>, int32 >_squadList;


	//패스, 스쿼드, 스포너 이 3개가 각각 하나씩 짝을이뤄 시작부터 끝까지 사용된다. 인터벌이 큰 틱을 통해 전멸한 스쿼드를 패스의 시작점에서 복구한다.
	UPROPERTY()
	TArray<class AEnemySquad*> _squadPool;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	TArray<class AReinforceSquadSpawner* > _spawners;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	TArray<class ACPatrolPath*> _paths;

	

	


};
