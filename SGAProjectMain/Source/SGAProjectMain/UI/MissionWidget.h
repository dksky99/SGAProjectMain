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
	virtual void NativeConstruct() override;
	void AddMissionSlot(UTexture2D* texture, FText name, FName ID);
	void SetMissionCompleted(FName ID);
	void ShowMissionCompletedText();

protected:
	void OnMissionCompleted();

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* _missionSlots;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _completedText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UMissionSlotWidget> _slotWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Game/UI")
	class UTexture2D* _planeMissionIcon;
};
