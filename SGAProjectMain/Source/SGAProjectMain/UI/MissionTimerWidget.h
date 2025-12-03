// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MissionTimerWidget.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UMissionTimerWidget : public UUserWidget
{
	GENERATED_BODY()
	
	void NativeConstruct() override;

protected:
	void HandleTimerUpdated(float remainingTime);

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _timerText;
};
