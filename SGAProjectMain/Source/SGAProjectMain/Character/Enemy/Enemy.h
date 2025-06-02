// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../CharacterBase.h"
#include "Enemy.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AEnemy : public ACharacterBase
{
	GENERATED_BODY()
	
public:
	AEnemy(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	virtual float TakeDamage(float damageAmount, FDamageEvent const& damageEvent, AController* eventInstigator, AActor* damageCauser) override;

private:
	UPROPERTY()
	class UEnemyStatComponent* _statComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/UI", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* _hpBarWidget;
};
