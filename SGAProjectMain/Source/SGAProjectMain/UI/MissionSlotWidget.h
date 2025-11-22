// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MissionSlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UMissionSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitializeSlot(UTexture2D* texture, FText name, FName ID);
	void DeactivateSlot();

	FName GetMissionID() const { return _missionID; }

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _missionNameText;

	//UPROPERTY(meta = (BindWidget))
	//class UTextBlock* _descriptionText;

	UPROPERTY(meta = (BindWidget))
	class UImage* _missionIcon;

	UPROPERTY()
	FName _missionID;
};
