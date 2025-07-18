// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../CharacterBase.h"
#include "Enemy.generated.h"


UENUM(BlueprintType)
enum class EUnitState : uint8
{
	Stay UMETA(DisplayName = "Stay"),				//가만히 있는다. 진짜 가만히
	Patrol UMETA(DisplayName = "Patrol"),			//비 전투상황에 특정 지점을 향해 이동하는 상태.
	InBattle UMETA(DisplayName = "InBattle"),		//전투 상황. 적을 발견하면 추적하고 공격한다.
	MAX
};

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


	EUnitState _unitState =EUnitState::Patrol;

};
