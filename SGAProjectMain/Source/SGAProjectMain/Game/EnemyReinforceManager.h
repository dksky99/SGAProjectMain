// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyReinforceManager.generated.h"

/*
	증원매니저다.
	증원은 최대 2번의 증원 카운트를 축적할수있고 하나의 증원을 호출하면 증원하나를 소모하고 새로 추가되는데 3분이 필요하다.
	증원가능횟수가 0이거나 증원후 1분이 지나지않으면 카운트가 남아있어도 증원이 불가하다.



*/

UCLASS()
class SGAPROJECTMAIN_API AEnemyReinforceManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemyReinforceManager();

	//이 함수를 통해 증원을 요청.
	bool TryReinforceCall(FVector callPoint,FVector targetPoint,AActor* target=nullptr);

	// 임무에 필요해서 증원을 한다면 증원 가능상태를 무시하고 증원을 부를 수있음.
	bool TryMissionCall(FVector callPoint,FVector targetPoint,AActor* target=nullptr);



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	//초기에 증원부대를 풀링.
	virtual void Init();

	class ASquadSpawner* GetExtraSpawner();

	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//여분의 스쿼드를 증원받은곳에 호출
	class AEnemySquad* GetExtraCallableSquad();

	void SetSpawner(FVector callPoint, class AEnemySquad* squad, class ASquadSpawner* spawner);

	void SetNextCallableTimer();
	void SetCallCountTimer();

	void AddCallableCount();
	void SetReinforceCallable();

	bool IsCallable();

protected:

	FTimerHandle _callCountTimer;
	FTimerHandle _nextReinforceTimer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	float _countAddTime = 180.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	float _nextReinforceCallTime = 60.f;

	bool _isCallable = true;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	int32 _maxCallAbleCount=2;

	int32 _callAbleCount=2;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	TMap< TSubclassOf<class AEnemySquad>, int >_squadList;



	UPROPERTY()
	TArray<class AEnemySquad*> _squadPool;


	UPROPERTY()
	TArray<class AReinforceSquadSpawner*> _spawnerPool;

};
