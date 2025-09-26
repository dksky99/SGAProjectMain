// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Enemy.h"
#include "Enemy_Standard.generated.h"

/**
 * 기본형은 중거리에 상대에 다가가기위해 너무 멀리돌아가야하거나 갈수없는위치에있다면 땅파기로 적의 위치에 나타나는 기능을 가짐.
 * 공격방식은 두가지로 하나는 적에게하는 공격, 방해되는 장애물을 박살내는 공격(겸사겸사 피격시 피해)
 */
UCLASS()
class SGAPROJECTMAIN_API AEnemy_Standard : public AEnemy
{
	GENERATED_BODY()

public:

	AEnemy_Standard(const FObjectInitializer& ObjectInitializer);


	

	virtual bool CheckAbleTryNear(AActor* target) override;
	virtual bool CheckAbleTryMiddle(AActor* target) override;
	virtual bool CheckAbleTryFar(AActor* target) override;

	virtual bool TryNear(AActor* target)   override;
	virtual bool TryMiddle(AActor* target) override;
	virtual bool TryFar(AActor* target)    override;
	bool TryCalling(AActor* target);
	void CallingReinforce();
	bool TryBurrow(AActor* target);
	void BurrowIn();
	void BurrowOut();



protected:


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* _burrowOut_Animation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* _burrowIn_Animation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* _callReinforce_Animation;
	FVector _burrowOutLoc;
	FTimerHandle _burrowTimer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	float _burrowOutDelay = 2.0f;

	AActor* _enemyTarget=nullptr;

	bool _hasReinforceAuthority=false;
};
