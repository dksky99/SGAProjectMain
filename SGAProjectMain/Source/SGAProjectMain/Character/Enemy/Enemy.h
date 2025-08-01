// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../CharacterBase.h"
#include "Enemy.generated.h"


UENUM(BlueprintType)
enum class EUnitState : uint8
{
	Stay UMETA(DisplayName = "Stay"),					//가만히 있는다. 진짜 가만히
	Patrol UMETA(DisplayName = "Patrol"),				//비 전투상황에 특정 지점을 향해 이동하거나 주둔지에서 주변을 정찰.
	Weak_Alert UMETA(DisplayName = "Weak_Alert"),		//약간의 소리가 감지되어 얕은 경계상태. 달리는 소리, 총성,폭발음 등 그게 들린그 위치로 이동해봄.
	Strong_Alert UMETA(DisplayName = "Strong_Alert"),	//포인트가 찍혀서 그위치로 가면서 적을 탐색한다.아직 적을 확인하진 않았다. 
	InBattle UMETA(DisplayName = "InBattle"),			//전투 상황. 적을 발견한 상태고 적이 죽을때까지 추적하고 공격한다.
	MAX
};

/**
 * 
 */
UCLASS(Blueprintable,BlueprintType)
class SGAPROJECTMAIN_API AEnemy : public ACharacterBase
{
	GENERATED_BODY()
	
public:
	AEnemy(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	//virtual float TakeDamage(float damageAmount, FDamageEvent const& damageEvent, AController* eventInstigator, AActor* damageCauser) override;

	virtual void PossessedBy(AController* NewController) override;

	virtual void UnPossessed() override;

	// 시야감각 측에서 적을 찾아서 배틀상태로 변환.
	virtual void FoundTarget(class ACharacterBase* target);

	//주변의 아군들에게 강한 경보상태로 만들고 다 같이 자신이 
	virtual void RaiseAlert();

	virtual void Spawn();
	bool IsReadyToSpawn();
	void ReadyToSpawn();

	bool AddToSquad(class AEnemySquad* temp);

	void UnitDeactivate();

	virtual void Dead() override;
	virtual void SpawnGhost() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/UI", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* _hpBarWidget;

private:

	FTimerHandle _respawnTimer;

	bool _isReadyToSpawn = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	float _respawnCoolDown;
	EUnitState _unitState =EUnitState::Patrol;

	TArray<class ACharacterBase*> _targets;


	UPROPERTY()
	TWeakObjectPtr<class AEnemySquad> _squad;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* _spawnMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* _deadMontage;
};
