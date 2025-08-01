// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../EnemySquad.h"
#include "ReinforcementSquad.generated.h"




UCLASS()
class SGAPROJECTMAIN_API AReinforcementSquad : public AEnemySquad
{
	GENERATED_BODY()
	
public:


	virtual void Init() override;

	bool CheckAbleToCall(FVector origin);

	void CallReinforcement();


	
	FVector GetCallPoint(FVector origin);

	void ReadyToCall();


protected:
	FTimerHandle _reinforcementTimer;
	FTimerHandle _callUnitTimer;

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
