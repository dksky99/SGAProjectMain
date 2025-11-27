// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy_Warrior.h"
#include "Enemy_HiveGuard.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AEnemy_HiveGuard : public AEnemy_Warrior
{
	GENERATED_BODY()

public:





	virtual bool CheckAbleTryMiddle(AActor* target) override;

	virtual bool TryMiddle(AActor* target) override;

	bool StartGuard();

	void GuardContinue();
	//하이브가드는 워리어와 부위 효과를 동일하게 가져간다 

protected:



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* _guardMontage;

};
