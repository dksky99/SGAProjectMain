// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SquadSpawner.h"
#include "ReinforceSquadSpawner.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AReinforceSquadSpawner : public ASquadSpawner
{
	GENERATED_BODY()
	
public:
	virtual void ActivateSpawner(class AEnemySquad* squad, FVector loc) override;

protected:
	//


	virtual void CallFinishAction() override;

};
