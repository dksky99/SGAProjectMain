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
	void InitializeStgSlot(const class AStratagem* stg);

	void UpdateStgSlot(const TArray<FKey>& inputBuffer);

	void ResetSlot();

private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _stgNameText;

	UPROPERTY(meta = (BindWidget))
	class UImage* _stgIcon;

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* _stgCommands;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/UI", meta = (AllowPrivateAccess = "true"))
	class UTexture2D* _leftArrow;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/UI", meta = (AllowPrivateAccess = "true"))
	class UTexture2D* _rightArrow;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/UI", meta = (AllowPrivateAccess = "true"))
	class UTexture2D* _upArrow;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/UI", meta = (AllowPrivateAccess = "true"))
	class UTexture2D* _downArrow;
};
