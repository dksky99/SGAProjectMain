// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySquad.generated.h"


UENUM(BlueprintType)
enum class ESquadState : uint8
{
	Stationed UMETA(DisplayName = "Stationed"), //특정 지역에 주둔함. 특정 포인트, 기지, 이런곳에 있는상태.
	Patrol UMETA(DisplayName = "Patrol"), // 배회함 그냥 랜덤한 지점을 돌아다니다님.
	Search UMETA(DisplayName = "Search"),	//탐색. 특정 위치를 향해 가면서 적을 확인.
	Attack UMETA(DisplayName = "Attack"), // 적이 확인됨. 적이 있는 위치로 이동.
	Deactivate UMETA(DisplayName = "Deactivate"), //비활성화. 개개인으로 다니는 몹들이 지원을 호출하기 전의 상태나 파괴되어 더이상 유닛을 생산못하는 기지의 상태.
	MAX
};



UCLASS()
class SGAPROJECTMAIN_API AEnemySquad : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemySquad();

protected:


	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Init();


	void UnitSpawnAct(class AEnemy* unit);
	// 이건 만들어놓고 어떻게 써야할지 모르겠다 놔두다 필요하면 쓰고 필요없으면 버리자
	virtual void Command_Search();

	// 이 스쿼드는 주둔 스쿼드 상태다. 병력들은 소환되고 그냥 주변에서 대기하며 전투를 기다린다. 
	virtual void Command_Stationed();

	// 이 스쿼드는 정찰 스쿼드다. 소환되면 가지고있는 경로를따라 순찰시키도록해야한다.
	virtual void Command_Patrol();

	// 이 스쿼드는 증원 스쿼드다 그중에 타겟을 받았고 소환되자마자 경계치를 최대로 올리고 정해진 지점을 향해 공격을 시작한다.
	virtual void Command_Attack();

	// 스쿼드 비활성화. 이 스쿼드는 당장 목표가 사라졌다. 주둔 부대였다면 주둔지가 다 파괴되어 더이상 유닛을 뽑아낼 수 없고
	// 증원스쿼드나 임무스쿼드 라면 이미 헬다이버를 격살하여 목표가 사라졌다. 
	//
	virtual void Command_Deactivate();

	//살아있는 병력들을 되돌려보내는 작업. 증원부대나 미션부대가 목표를달성한후 다시 필요한곳에 소환되기 위해 다시 돌아간다.
	void ReturnToSquad();


	//소환을 위한 함수들
	//-------------------------------------
	//특정 병종의 유닛을 골라서 뽑아낸다.
	class AEnemy* GetUnitFromPool(TSubclassOf<AEnemy> EnemyClass);
	//유닛을 풀에 되돌려놓는다.
	void ReturnUnitToPool(AEnemy* Enemy);
	//자신에 속한 모든 병력을 풀로 되돌린다.
	void DeactivateAllUnits();
	// 풀 안에 소환되지 않고 남은 유닛이 있는지 확인.
	class AEnemy* CheckExtraUnit();

	//활성화된 스쿼드인지. 매니저쪽에서 이 스쿼드의 상태를 확인하기위한 함수다. 단 하나의 유닛이라도 활성화된 상태라면 활성화된 스쿼드로 본다.
	bool IsActivatedSquad();

	//현재 활성화상태인 유닛의 수를 샌다.
	int32 CheckActivateUnitCount();


protected:
	//내부 체크용 함수. 활성화중인 유닛인지, 살아있는 유닛인지.
	bool IsActivatedUnit(class AEnemy* unit);
	bool IsAliveUnit(class AEnemy* unit);

	//
	FVector MakeRandomLocation();
protected:

	UPROPERTY()
	TArray<class AEnemy*> _unitPool;


	//이 부대의 병종과 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	TMap< TSubclassOf<class AEnemy>,int >_unitList;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game/Spawn", meta = (AllowPrivateAccess = "true"))
	USceneComponent* _rootComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game/Spawn", meta = (AllowPrivateAccess = "true"))
	USceneComponent* _spawnPoint;

	//스쿼드의 상태. 이에따라 소환시 공유받는 명령이 달라진다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game/Squad", meta = (AllowPrivateAccess = "true"))
	ESquadState _squadState = ESquadState::Deactivate;

	//스쿼드의 공통된 타겟. 만약 한 병사가 타겟을 얻으면 모두가 타겟을 공유한다. 
	UPROPERTY()
	class AActor* _target;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game/Squad", meta = (AllowPrivateAccess = "true"))
	float _targetLocRadius=200.f;

	//공통된 목표점. 증원직후에 타격해야할 목표점이있다면 이 지점을 공유받는다.
	UPROPERTY()
	FVector _targetLoc;

	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	class ACPatrolPath* _patrolPath;
};
