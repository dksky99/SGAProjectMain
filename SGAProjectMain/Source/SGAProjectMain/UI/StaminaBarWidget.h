// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StaminaBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UStaminaBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetStamina(float ratio, bool isRecover);

private:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* _curStaminaPB;

	bool _canBlink = false;
	
};
