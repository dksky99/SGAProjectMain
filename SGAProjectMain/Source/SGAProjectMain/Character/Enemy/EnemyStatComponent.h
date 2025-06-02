// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../StatComponent.h"
#include "EnemyStatComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FHpChanged, float);

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UEnemyStatComponent : public UStatComponent
{
	GENERATED_BODY()

public:
	void ChangeHp(float amount) override;
	
	FHpChanged _hpChanged;
};
