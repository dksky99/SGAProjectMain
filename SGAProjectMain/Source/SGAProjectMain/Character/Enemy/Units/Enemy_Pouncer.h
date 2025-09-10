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



	virtual bool TryNear(AActor* target)   override;
	virtual bool TryMiddle(AActor* target) override;
	virtual bool TryFar(AActor* target)    override;



	bool Jump(AActor* target);
	bool Jump(FVector target);

	bool CalculateLaunchDirection(const FVector& Start, const FVector& Target, float Speed, FVector& OutLaunchVelocity);

	bool CalculateLaunchDirectionWithTime(const FVector& Start, const FVector& Target, float Speed, FVector& OutLaunchVelocity, float& OutFlightTime);
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/AttackCollision", meta = (AllowPrivateAccess = "true"))
	class UCapsuleComponent* _claw_L;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/AttackCollision", meta = (AllowPrivateAccess = "true"))
	class UCapsuleComponent* _claw_R;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* _jump_Animation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Jump", meta = (AllowPrivateAccess = "true"))
	float _jumpPower=1000.f;




};
