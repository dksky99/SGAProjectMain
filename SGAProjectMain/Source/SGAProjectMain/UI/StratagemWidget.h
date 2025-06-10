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
	void InitializeStgWidget(const TArray<struct FStratagemSlot>& stgSlots);

	void UpdateStgWidget(int32 index, const TArray<FKey> inputBuffer);

private:
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* _stgSlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UStratagemSlotWidget> _slotWidgetClass;
};
