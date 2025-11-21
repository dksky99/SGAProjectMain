// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../EnemySquad.h"
#include "StationedSquad.generated.h"

/**
 * 주둔 병력 :  주변에 헬다이버가 접근시 유닛을 쏟아낸다. 탐지는 활성화되있고 살아있는 헬다이버가 일정거리 내에 접근시 
 * 주기가 느린 틱을 통해 해보도록하자.
 * 그리고 하나의 주둔 스쿼드내에 여러 생산건물을 할당하여 생산건물은 이 풀내에서 유닛을 뽑아서소환하도록한다.
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AStationedSquad : public AEnemySquad
{
	GENERATED_BODY()
	
public:
	AStationedSquad();

	virtual void BeginPlay() override;
	UFUNCTION()
	void RecieveDamage(float damage,float armorPen);
	void CallRemainUnit();
	void ActivateFactory();
	void DestroyFactory();

protected:

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
protected:

	FTimerHandle _GenerateTimer;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	float _generateCoolDown = 5.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	float _stationRadius = 1000.f;
	bool _isGeneratable = true;

	bool _isActivating = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	int _defense = 0.f;//나중엔 관통력이 이 미만이면 피해를 피해가 안들어옴

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	int _maxDurability = 1.f;
	int _curDurability = 1.f;



	//고정된 생산지는 전부 파괴가 가능하다. 이떄 이 박스컴포넌트가 파괴의 트리거가 될예정. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	class UShapeComponent* _destroyPoint=nullptr;


};
