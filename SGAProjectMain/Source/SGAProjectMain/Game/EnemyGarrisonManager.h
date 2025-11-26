// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyGarrisonManager.generated.h"


/*
	주둔지를 관리하는 매니저. 
	주둔지는 하나의 기지에 주둔하는 스쿼드와 그 스쿼드와 연결된 여러 팩토리스포너로 이뤄져있다. 

	이 매니저의 목적은 미션세팅과 스쿼드의 초기화를 담당해줄 예정이다. 아무래도 지금은 직접 레벨에 있는 스쿼드와 팩토리들을 연결해줘야할듯하다. 
	스쿼드와 집결포인트와 스포너리스트가 하나의 세트가 될것이다.

*/



USTRUCT(BlueprintType)
struct FGarrisonData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad")
	class AEnemySquad* Squad;

	// 2. 배열 (FString 배열, 예: 아이템 목록, 태그 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad")
	TArray<class AFactorySquadSpawner* > Spawners;

	// 3. 좌표 (FVector, 3차원 위치나 방향)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad")
	AActor* RellyPoint;

	void AddDestroyedCount() { _destroyedCount++; }

	int32 GetDestroyedCount() { return _destroyedCount; }
	
	//전부 파괴시 임무가있다면 보상.
	UPROPERTY(EditAnywhere)
	FName _objectiveID;

protected: 
	int32 _destroyedCount = 0;



};

UCLASS()
class SGAPROJECTMAIN_API AEnemyGarrisonManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemyGarrisonManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void InitGarrison();

	void CheckFactoryDestroyed(const class AEnemySquad* squad);
	
	void CheckMissionComplete(const FGarrisonData* const data);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	TArray<FGarrisonData> _garrisonDatas;
	
};
