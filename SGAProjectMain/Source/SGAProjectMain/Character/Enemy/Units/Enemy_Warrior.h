// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy_Standard.h"
#include "Enemy_Warrior.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AEnemy_Warrior : public AEnemy_Standard
{
	GENERATED_BODY()

public:

	AEnemy_Warrior(const FObjectInitializer& ObjectInitializer);




	virtual bool CheckAbleTryMiddle(AActor* target) override;
	virtual bool TryMiddle(AActor* target) override;


	virtual void ResetUnit() override;

protected:

	UFUNCTION()
	void Warrior_Bleeding();
	void CalcBleedingDamage();
	virtual void PartInit() override;

protected:


	FTimerHandle _bleedingTimer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Ability/Bleeding", meta = (AllowPrivateAccess = "true"))
	float _additiveHP = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Ability/Bleeding", meta = (AllowPrivateAccess = "true"))
	float _bleedingDamage = 70.f;

	bool _isBleeding = false;




};
