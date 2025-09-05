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



protected:

	class UCapsuleComponent* _claw_L;


	class UCapsuleComponent* _claw_R;


};
