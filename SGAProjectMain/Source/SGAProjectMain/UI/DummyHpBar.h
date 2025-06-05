// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DummyHpBar.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UDummyHpBar : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetHp(float ratio);

private:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* _curHpPB;
};
