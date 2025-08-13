// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlaneAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UPlaneAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void PlayMontageFromSection(UAnimMontage* montage, FName sectionName);

	UFUNCTION()
	void AnimNotify_OpenHatch();

	UFUNCTION()
	void AnimNotify_Exit();
};
