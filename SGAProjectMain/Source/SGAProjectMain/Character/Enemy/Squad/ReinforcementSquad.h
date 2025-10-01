// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../EnemySquad.h"
#include "ReinforcementSquad.generated.h"


/*
	일부 유닛들이 증원을 요청할 시 소환되는 병력들. 5분의 재 소환 쿨타임을 가지고있고 특정 포인트가 찍히면 소환가능지역인지 확인 후 소환. 





*/

UCLASS()
class SGAPROJECTMAIN_API AReinforcementSquad : public AEnemySquad
{
	GENERATED_BODY()
	
public:


	virtual void Init() override;

	bool CheckAbleToCall(FVector origin,FVector target);

	void CallReinforcement();

	bool SetTargetLoc(FVector target);

	
	FVector GetCallPoint(FVector origin);

	void ReadyToCall();


protected:
	FTimerHandle _reinforcementTimer;
	FTimerHandle _callUnitTimer;

	FVector _targetLoc;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	float _coolDown=300.0f;

	bool _isReadyToCall = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	float _callRadius=1000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	float _callingDelay_Min=0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	float _callingDelay_Max=3.f;





};
