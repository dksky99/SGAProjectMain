// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Enemy.h"
#include "Enemy_Standard.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AEnemy_Standard : public AEnemy
{
	GENERATED_BODY()

public:

	AEnemy_Standard(const FObjectInitializer& ObjectInitializer);

	bool AttackMelee();

	void ActivateClaw_L();
	void ActivateClaw_R();
	void DeactivateClaw_L();
	void DeactivateClaw_R();


protected:
	class UCapsuleComponent* _claw_L;


	class UCapsuleComponent* _claw_R;

};
