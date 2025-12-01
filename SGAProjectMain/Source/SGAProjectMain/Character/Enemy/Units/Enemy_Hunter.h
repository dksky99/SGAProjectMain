// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy_Pouncer.h"
#include "Enemy_Hunter.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AEnemy_Hunter : public AEnemy_Pouncer
{
	GENERATED_BODY()

public:





	virtual bool CheckAbleTryFar(AActor* target) override;
	virtual bool TryFar(AActor* target)    override;

	bool TryDodge(AActor* target);


protected:

	virtual void PartInit() override;
protected:

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* _dodgeMontage;

};
