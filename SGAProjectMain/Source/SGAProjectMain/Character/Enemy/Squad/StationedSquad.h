// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../EnemySquad.h"
#include "StationedSquad.generated.h"

/**
 * 주둔 병력 :  이 부대는 생산기지에 있는 생산 건물 하나에 해당. 기본적으로 한번에 유지할 수있는 병력의 수는 등록된 전체 병력의 절반. 
 * 유닛당 사망시 2분의 쿨타임이있고
 * 부대는 활동중 병력의 절반 미만이 되면 5초에 하나씩 증원 한다. 한 부대당 병력의 수는 4~6마리 수준. 
 * 파괴시 더이상 생산되지 않음. : 결국 미션이 진행될수록 병력이 낭비되는 느낌이다 추후에 각 스쿼드는 
 * 해당된 유닛을 들고있는게아니라 통합매니저로부터 받아오는 형식으로 바꾸는것도 최적화를 위해 좋을것같기도 하다?
 */
UCLASS()
class SGAPROJECTMAIN_API AStationedSquad : public AEnemySquad
{
	GENERATED_BODY()
	
public:
	AStationedSquad();

	virtual void BeginPlay() override;

	void CallRemainUnit();
	void ActivateFactory();
	void DestroyFactory();

protected:

protected:

	FTimerHandle _GenerateTimer;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	float _generateCoolDown = 5.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	float _stationRadius = 1000.f;
	bool _isGeneratable = true;

	bool _isActivating = true;



	//고정된 생산지는 전부 파괴가 가능하다. 이떄 이 박스컴포넌트가 파괴의 트리거가 될예정. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* _destroyPoint;


};
