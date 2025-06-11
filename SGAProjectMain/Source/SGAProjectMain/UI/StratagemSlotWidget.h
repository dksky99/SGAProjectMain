// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StratagemSlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UStratagemSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeSlot(const class AStratagem* stg);
	void ResetSlot();

	void UpdateSlot(int32 comboNum);

	void DimSlot();

	void SetSlotOpacity(float opacity);

private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _stgNameText;

	UPROPERTY(meta = (BindWidget))
	class UImage* _stgIcon;

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* _stgCommands;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/UI", meta = (AllowPrivateAccess = "true"))
	class UTexture2D* _arrow; // ¿À¸¥ÂÊ
};
