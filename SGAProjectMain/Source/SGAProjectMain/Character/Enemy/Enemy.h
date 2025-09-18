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


UENUM(BlueprintType)
enum class EBattleState : uint8
{
	None UMETA(DisplayName = "None"),				//타겟이 없다.
	Far UMETA(DisplayName = "Far"),					//매우 멀다. 타겟의 위치로 이동을 해야한다.
	Middle UMETA(DisplayName = "Middle"),				//스킬 시전거리 내. 차저라면 돌격을 시전할것이고 헌터계열은 도약을 쓸것. 기술의 사용조건이 안됬다면 근접공격거리까지 가까이 가게될것. 
	Near UMETA(DisplayName = "Near"),		//매우 근접. 기본공격이 닿는 지점.
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


	class UPatrolComponent* GetPatrol() { return _patrolComponent; }
	virtual void Dead() override;
	virtual void SpawnGhost() override;

	

	EUnitState GetUnitState() { return _unitState; }
	void SetUnitState(EUnitState state);
	void SetStay();
	void SetPatrol();
	void SetWeak_Alert();
	void SetStrong_Alert();
	void SetInBattle();

	virtual bool TryNear(AActor* target) { return false; }
	virtual bool TryMiddle(AActor* target) { return false; }
	virtual bool TryFar(AActor* target) { return false; }


protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/UI", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* _hpBarWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Patrol", meta = (AllowPrivateAccess = "true"))
	class UPatrolComponent* _patrolComponent;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* _spawnMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* _reinforcementMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* _deadMontage;

private:

	FTimerHandle _respawnTimer;

	bool _isReadyToSpawn = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	float _respawnCoolDown;





	TArray<class ACharacterBase*> _targets;

	EUnitState _unitState=EUnitState::Stay;
	UPROPERTY()
	TWeakObjectPtr<class AEnemySquad> _squad;


};
