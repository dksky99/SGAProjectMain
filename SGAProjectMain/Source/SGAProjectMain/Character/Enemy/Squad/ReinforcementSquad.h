// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../EnemySquad.h"
#include "ReinforcementSquad.generated.h"


/*
	일부 유닛들이 증원을 요청할 시 소환되는 병력들. 5분의 재 소환 쿨타임을 가지고있고 특정 포인트가 찍히면 소환가능지역인지 확인 후 소환. 
	 
	여기서 호출된 병력은 상태가 Stay상태가 되면 다시 돌아가서 증원을 기다린다.(적을 전부 척살하고 더이상 자극이 없을경우 그자리에 있을필요가없으니 복귀)




*/



UCLASS()
class SGAPROJECTMAIN_API AReinforcementSquad : public AEnemySquad
{
	GENERATED_BODY()
	
public:


	virtual void Init() override;

	//소환가능한지 확인. 이떄 소환가능한 포인트를반환. 
	bool CheckAbleToCall(FVector origin,FVector& recieve);

	bool CallReinforcement(FVector target);
	//병력을 하나씩 소환하고 일정쿨타임후에 다시 소환. 더이상 풀에 남아있는 소환가능한 유닛이 없을떄까지 진행.
	void CallUnit();

	//다음 소환위치를 설정. 
	bool SetTargetLoc(FVector target);

	
	FVector GetCallPoint(FVector origin);
	//모든 유닛이 소환가능한상태로 전환.
	void ReadyToCall();


protected:
	FTimerHandle _reinforcementTimer;
	FTimerHandle _callUnitTimer;

	FVector _targetLoc;
	
	//재 증원의 주기. 이 시간이 다 차지 않으면 유닛이 전부 돌아왔어도 호출이불가.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	float _coolDown=300.0f;

	bool _isReadyToCall = true;

	//증원을 부를떄 이 크기의 범위 내에서 랜덤한위치에 유닛이 소환됨.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	float _callRadius=1000.f;
	
	//유닛증원이 된후 다음 유닛이 증원될때까지 걸리는 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	float _callingDelay_Min=0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	float _callingDelay_Max=3.f;





};
