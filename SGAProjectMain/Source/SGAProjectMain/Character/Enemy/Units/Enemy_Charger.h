// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Enemy.h"
#include "Enemy_Charger.generated.h"

/**
 * 차징을 시작할때 콜리전과 데미지가 부여되고 충돌액터 리스트를 주기적으로 초기화해주든 어떻게든 해야할듯
 * 차징이끝날떄 콜리전을 비활성화 
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AEnemy_Charger : public AEnemy
{
	GENERATED_BODY()

public:

	AEnemy_Charger(const FObjectInitializer& ObjectInitializer);




	virtual bool CheckAbleTryNear(AActor* target) override;
	virtual bool CheckAbleTryMiddle(AActor* target) override;
	virtual bool TryNear(AActor* target)   override;
	virtual bool TryMiddle(AActor* target) override;
	virtual bool TryFar(AActor* target)    override;

	void StartCharging();

	void FinishCharging();

	void CancelCharging();

	void SetDefaultState();
	void SetChargingState();

protected:


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	class UUnitAttackDataAsset* _chargingAttackData;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* _chargingFinishMontage;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* _chargingCanceledMontage;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stat", meta = (AllowPrivateAccess = "true"))
	float _chargingMoveSpeed=1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stat", meta = (AllowPrivateAccess = "true"))
	float _chargingMaxRotateSpeed = 50.f;
};
