// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Enemy.h"
#include "Enemy_Pouncer.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AEnemy_Pouncer : public AEnemy
{
	GENERATED_BODY()

public:

	AEnemy_Pouncer(const FObjectInitializer& ObjectInitializer);




	virtual bool CheckAbleTryNear(AActor* target) override;
	virtual bool CheckAbleTryMiddle(AActor* target) override;
	virtual bool TryNear(AActor* target)   override;
	virtual bool TryMiddle(AActor* target) override;
	virtual bool TryFar(AActor* target)    override;



	bool JumpAttack(AActor* target);
	bool JumpAttack(FVector target);

	bool CalculateLaunchDirection(const FVector& Start, const FVector& Target, float Speed, FVector& OutLaunchVelocity);

	bool CalculateLaunchDirectionWithTime(const FVector& Start, const FVector& Target, float Speed, FVector& OutLaunchVelocity, float& OutFlightTime);
protected:



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	class UUnitAttackDataAsset* _jumpAttackData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Jump", meta = (AllowPrivateAccess = "true"))
	float _jumpPower=1000.f;




};
