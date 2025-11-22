// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SquadSpawner.h"
#include "FactorySquadSpawner.generated.h"

/**
 * 팩토리스포너는 스포너인 동시에 파괴가능한 건물이다.
 * 보통 콜리전은 2가지를 갖고 태그를 2가지 갖고있는다. 
 * 파괴방식은 다양한데 일격에 높은 철거수치를 꽂거나 다른 유닛들처럼 hp를깎는 방식 이렇게 2가지가있다.
 * 파괴조건을 각자 다르게 할수 있게 하자. 
 * 팩토리스포너는 헬다이버가주변 일정범위 내에 도달하면 유닛을 소환하기시작한다.
 * 주기가 상당히 긴 틱방식으로 헬다이버의 접근을 확인토록하자. 플레이어의 컨트롤러를가져오는 기능을써보자. 
 * 거기서 GetPawn을 쓰면 그게 왠만하면 헬다이버일것이다.
 */
UCLASS()
class SGAPROJECTMAIN_API AFactorySquadSpawner : public ASquadSpawner
{
	GENERATED_BODY()
public:
	AFactorySquadSpawner();
protected:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void CheckDistanceToTarget();

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual void ProcessDamage(const struct FCDamageEvent* damageEvent);

	void DestroyFactory();
protected:


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Spawn", meta = (AllowPrivateAccess = "true"))
	float _alertRange = 3000.f;




};
