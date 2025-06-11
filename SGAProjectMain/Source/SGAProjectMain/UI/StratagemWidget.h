// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StratagemWidget.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UStratagemWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitializeWidget(const TArray<struct FStratagemSlot>& stgSlots);

	void UpdateWidget(int32 stgIndex, int32 comboNum, bool bPrefixMax);

	void ResetWidget();

private:
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* _stgSlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UStratagemSlotWidget> _slotWidgetClass;
};
