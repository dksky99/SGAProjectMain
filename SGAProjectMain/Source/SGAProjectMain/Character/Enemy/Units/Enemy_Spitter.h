// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Enemy.h"
#include "Enemy_Spitter.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AEnemy_Spitter : public AEnemy
{
	GENERATED_BODY()

public:

	AEnemy_Spitter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	//추후에 투사체에 오브젝트 풀링을 추가할때 사용해보자
	void Init_Pooling();


	virtual bool CheckAbleTryNear(AActor* target) override;
	virtual bool CheckAbleTryMiddle(AActor* target) override;
	virtual bool TryNear(AActor* target)   override;
	virtual bool TryMiddle(AActor* target) override;
	virtual bool TryFar(AActor* target)    override;

	virtual void AcidbagDestroyed();

	//스피터 계열의 토사 공격은 지속되는 광선같은느낌이아니라 연발의 오차가 있는 다연장 투사공격이였다. 투사체를 일정시간마다 조금식 오차를 갖고 뱉어내는것이면 충분할듯. 
	//스핏은 스킬을 시전하는 트리거이고
	bool Spit(AActor* target);
	//이건 각 토사를 투사하는 함수. Spit때 몽타주중 호출할 델리게이트에 바인드.
	void SpitProjectile();


	bool CalculateLaunchDirection(const FVector& Start, const FVector& Target, float Speed, FVector& OutLaunchVelocity);

	bool CalculateLaunchDirectionWithTime(const FVector& Start, const FVector& Target, float Speed, FVector& OutLaunchVelocity, float& OutFlightTime);


	virtual void ResetUnit() override;

protected:

	virtual void PartInit() override;

	void SpitReady() { _isSpitReady = true; }


protected:


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Attack", meta = (AllowPrivateAccess = "true"))
	class UExplosionComponent* _explosionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Attack", meta = (AllowPrivateAccess = "true"))
	class UUnitAttackDataAsset* _spitAttack_Animation;

	UPROPERTY()
	TArray<class AGunBulletBase*> _projectilePool;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Attack", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AGunBulletBase> _projectileClass;
	FVector _spitDirection;

	FTimerHandle _spitTimer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Ability/Spit", meta = (AllowPrivateAccess = "true"))
	float _spitCooldown= 3.0f;

	bool _isSpitReady = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Attack", meta = (AllowPrivateAccess = "true"))
	float _errorDegree = 0.0f;

	bool _isExplode = false;
	
};
