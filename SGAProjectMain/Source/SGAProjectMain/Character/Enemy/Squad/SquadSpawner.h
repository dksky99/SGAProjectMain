// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SquadSpawner.generated.h"
/*
	스쿼드와 스포너의 역할분배. 
	스쿼드는 유닛들을 통솔하는 역할만을,
	스포너는 스쿼드를 받아 그안에서 규칙에 맞게 유닛을 스쿼드에서 소환하는 역할을한다. 

	증원부대 :  스쿼드가 목표한 위치로 스포너를 이동하여 증원할 병력을 스포너가 소환한다.
	주둔부대 : 스쿼드가 여러대의 스포너를 보유하고 헬다이버가 이 스포너의 일정거리 이상 가까이왔다면 유닛들을 생산하기 시작.
	임무부대 : 임무를 수행중 반드시 소환되는 부대로 임무지에 배정되어있고 임무를 수행하다 특정 상황이오면 
			   임무지 주변 랜덤위치에서유닛을 증원과 비슷한 느낌으로 소환. 하지만 소환자가 따로 존재하지 않음.
	정찰부대 : 처음 시작될때 맵을 #자로 가로지르는 4개의 정찰로를 갖고 각각 하나의 부대씩 배정하여 보낸다.
			   경로의 시작지점에 배치되며 이후 유닛들을 차례로 뱉어낸다. 정찰병들이 전멸하고 유닛들이 전부 복구되면 다시 
			   시작점에서 병력을 생성하여 정찰병들을 보낸다.
	
	스쿼드는 분류를 나누지 않고 전부 그냥 EnemySquad로 한다. 소환방식은 이제 스포너가 결정캐 하자.
	이렇게하면 더이상 사용되지 못하는 스쿼드. 임무부대나 주둔부대가 사라지면 전부 증원부대로 몰아넣을수도있다.
	병력낭비를 덜을수 있을듯하다. 미션을 수행할수록 적의 증원병의 수가 늘어나는 난이도 증가의 효과가 있을것.

	사이클 : 
	증원, 정찰, 미션 : 매니저가 우선 지점을 받아서(그저 목표지점과 가까운 랜덤한 위치 혹은 송신자 혹은 정찰로의시작점) 그위치에 
	스쿼드 스포너를 스쿼드를 결속시켜 위치를 이동시킨 후 (이때 스포너의 인보커로인해 주변에 네비영역이 로딩될것) 스포너가 스쿼드로부터 
	비활성화된 유닛을 차례로 받아서 일정혹은 랜덤한 주기로 소환을 하게됨. 

	생산 : 스포너가 한번에 소환할수 있는 병력의 수가 정해져있고 한번소환하면 정해진 양의 유닛이 적당히 짧은 주기로 하나씩 정해진 양만큼 소환되고
	이후 적당히 긴 쿨타임후에 다시 소환할 수 있음. 주둔 부대하나에 여러개의 스쿼드 스포너를 상속받은 파괴가능한 건물들을 갖고있다고 보면된다. 
	
	스포너들이 전부 파괴되면 이 스쿼드를 통째로 증원 매니저에 옮겨버리도록하자. 

	



*/
UCLASS()
class SGAPROJECTMAIN_API ASquadSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASquadSpawner();


	//
	void ActivateSpawner(class AEnemySquad* squad);

	void DeactivateSpawner();

	// 스쿼드에 남아있는 병력을 부르는 함수. 하나씩 개별로 소환하기위해 사용. 
	void CallRemainUnit();

	void SpawnUnits();

	FVector GetSpawnPoint();
	
	void SpawnerCoolDownFinish();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	
public:	


protected:



	//유닛별 소환의 텀을 다루는 타이머
	FTimerHandle _spawnTimer;
	//스포너가 한번 소환이라는 행동을 했을시 다시 작동시키기 위한 타이머.
	FTimerHandle _spawnerTimer;

	//네비영역이 동적으로 호출되기때문에 유닛들이 소환될 좌표를 얻을 수 없다 그래서 소환전에 이 스포너를 그위치에 배치하여 
	//네비영역을 생성한 후에 그뒤에 소환을 한다. 생산건물 또한 이 클래스를 상속받을것이다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Nav", meta = (AllowPrivateAccess = "true"))
	class UNavigationInvokerComponent* _navInvokerComponent;

	UPROPERTY()
	class AEnemySquad* _squad=nullptr;

	//한번의 스폰 후 다음 스폰때까지 걸릴 텀
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Spawn", meta = (AllowPrivateAccess = "true"))
	double _minSpawnInterval = 0.3f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Spawn", meta = (AllowPrivateAccess = "true"))
	double _maxSpawnInterval = 1.f;
	//스포너가 한번 동작하면 스포너 자체가 갖고있을 쿨타임.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Spawn", meta = (AllowPrivateAccess = "true"))
	float _spawnerCoolTime=180.f;

	//한번에 소환가능한 병력 수.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Spawn", meta = (AllowPrivateAccess = "true"))
	int32 _maxSpawnCount = -1;

	int32 _curSpawnCount = 0;

	//스포너가 준비됬는지.
	bool _spawnerReady = true;

	//병력의 소환될범위의 오차. 생산건물이라면 오차가 없어야할것이고 증원이나 정찰이라면 오차가 좀 커야 자연스러울것이다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Spawn", meta = (AllowPrivateAccess = "true"))
	float _callRadius = 0.f;

	//소환되는 유닛의 yaw로테이션이 랜덤으로할것인가. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Spawn", meta = (AllowPrivateAccess = "true"))
	bool _randomYaw = false;


};
