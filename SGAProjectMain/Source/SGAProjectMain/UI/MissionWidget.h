// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MissionWidget.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UMissionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void AddMissionSlot(UTexture2D* texture, FString name);
	void ShowTempText();
	
protected:
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* _missionSlots;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _tempText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UMissionSlotWidget> _slotWidgetClass;
};
