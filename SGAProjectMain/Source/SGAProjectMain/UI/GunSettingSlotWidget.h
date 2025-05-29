// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GunSettingSlotWidget.generated.h"

/**
 * 
 */

UCLASS()
class SGAPROJECTMAIN_API UGunSettingSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitializeSlot(class UGunSettingSlotDataAsset* asset);

	void SetHighlight(bool isCurMode);

	FName GetOptionKey() { return _optionKey; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* _slotImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* _slotName;

private:
	FName _optionKey;

	bool _isHighlighted = false;
};
