// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySquad.generated.h"


UENUM(BlueprintType)
enum class ESquadState : uint8
{
	Stationed UMETA(DisplayName = "Statione"), //특정 지역에 주둔함. 특정 포인트, 기지, 이런곳에 있는상태.
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


protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	TArray<class AEnemy*> _unitPool;




};
