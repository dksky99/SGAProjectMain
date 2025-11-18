// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MissionResultWidget.generated.h"

/**
 * 
 */

UCLASS()
class SGAPROJECTMAIN_API UMissionResultWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitializeWidget(const struct FMissionResult& missionResult);

protected:
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* _rewardPanel;

	UPROPERTY(meta = (BindWidget))
	class UMissionResultSlotWidget* _mainObjSlot;

	UPROPERTY(meta = (BindWidget))
	class UMissionResultSlotWidget* _optionalObjSlot;

	UPROPERTY(meta = (BindWidget))
	class UMissionResultSlotWidget* _extractedSlot;

	UPROPERTY(meta = (BindWidget))
	class UMissionResultSlotWidget* _timeSlot;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _totalXpText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _totalRequisitionText;

	UPROPERTY()
	TArray<class UMissionResultSlotWidget*> _rewardSlots;
};
