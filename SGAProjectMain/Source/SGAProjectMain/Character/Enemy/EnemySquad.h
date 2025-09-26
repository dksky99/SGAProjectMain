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

USTRUCT()
struct FEnemyUnit
{
	GENERATED_BODY()

	UPROPERTY()
	TMap< TObjectPtr<class AEnemy>,TObjectPtr<class AEnemyController>>  _units;
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


	virtual void SpawnAllUnits();
	virtual bool SpawnUnit(TPair< TObjectPtr<class AEnemy>, TObjectPtr<class AEnemyController>>* unit);

	void UnitSpawnAct(TPair< TObjectPtr<class AEnemy>, TObjectPtr<class AEnemyController>>* unit);
	// 이건 만들어놓고 어떻게 써야할지 모르겠다 놔두다 필요하면 쓰고 필요없으면 버리자
	virtual void Command_Search();

	// 정해진 지점 주위를 순찰하도록.
	virtual void Command_Stationed();

	// 정해진 경로를 향해 순찰하도록
	virtual void Command_Patrol();

	// 경계치를 최대로 올리고 정해진 지점을 향해 공격
	virtual void Command_Attack();

	// 스쿼드 비활성화. 
	virtual void Command_Deactivate();

	TPair< TObjectPtr<class AEnemy>, TObjectPtr<class AEnemyController>>* GetUnitFromPool(TSubclassOf<AEnemy> EnemyClass);
	void ReturnUnitToPool(AEnemy* Enemy);
	void DeactivateAllUnits();
	// 풀 안에 소환되지 않고 남은 유닛이 있는지 확인.
	TPair< TObjectPtr<class AEnemy>, TObjectPtr<class AEnemyController>>* CheckExtraUnit();


	int32 CheckActivateUnitCount();


protected:
	bool IsActivatedUnit(TPair< TObjectPtr<class AEnemy>, TObjectPtr<class AEnemyController>>* unit);

	FVector MakeRandomLocation();
protected:

	UPROPERTY()
	TMap<TSubclassOf<class AEnemy>,FEnemyUnit> _unitPool;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	TMap< TSubclassOf<class AEnemy>,int >_unitList;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game/Spawn", meta = (AllowPrivateAccess = "true"))
	USceneComponent* _rootComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game/Spawn", meta = (AllowPrivateAccess = "true"))
	USceneComponent* _spawnPoint;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game/Squad", meta = (AllowPrivateAccess = "true"))
	ESquadState _squadState = ESquadState::Deactivate;

	UPROPERTY()
	class AActor* _target;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game/Squad", meta = (AllowPrivateAccess = "true"))
	float _targetLocRadius=200.f;
	UPROPERTY()
	FVector _targetLoc;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	class ACPatrolPath* _patrolPath;
};
