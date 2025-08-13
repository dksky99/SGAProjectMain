// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/HorizontalBox.h"
#include "CommandWidget.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EArrowEffectType : uint8
{
	Opacity,
	Color
};

UCLASS()
class SGAPROJECTMAIN_API UCommandWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void InitializeSlot(const TArray<FKey>& combo);
	virtual void ResetSlot();
	virtual void UpdateSlot(int32 comboNum);
	virtual void SetSlotOpacity(float opacity);
	virtual void OnCompleted();

protected:
	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* _commandArrows;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/UI", meta = (AllowPrivateAccess = "true"))
	class UTexture2D* _arrow; // 오른쪽

	UPROPERTY(EditAnywhere, Category = "Game/UI")
	EArrowEffectType _effectType = EArrowEffectType::Opacity; // 화살표 효과 타입

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* _widgetSwitcher;
};
