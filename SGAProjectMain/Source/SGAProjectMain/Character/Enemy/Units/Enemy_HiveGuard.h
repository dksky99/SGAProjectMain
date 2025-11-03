// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy_Standard.h"
#include "Enemy_HiveGuard.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AEnemy_HiveGuard : public AEnemy_Standard
{
	GENERATED_BODY()

public:





	virtual bool CheckAbleTryMiddle(AActor* target) override;

	virtual bool TryMiddle(AActor* target) override;

	bool StartGuard();

	void GuardContinue();


protected:



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* _guardMontage;

};
