// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommandWidget.h"
#include "StratagemSlotWidget.generated.h"

UENUM(BlueprintType)
enum class EStgSlotWgtState : uint8
{
	Normal,
	Operating,
	Cooldown
};
/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UStratagemSlotWidget : public UCommandWidget
{
	GENERATED_BODY()

public:
	void InitializeSlot(const class AStratagem* stg, class UStratagemWidget* parentWidget);
	void ResetSlot() override;

	void UpdateSlot(int32 comboNum) override;
	void SetCooldown(float remainingTime);

	void SetSlotDeactivatingState();
	void SetSlotOperatingState();
	void SetSlotCooldownState(float remainingTime);

	void SetSlotOpacity(float opacity) override;

	bool IsForcedShowing() { return _isForcedShowing; }
	void SetForcedShowing(bool isForcedShowing) { _isForcedShowing = isForcedShowing; }

	EStgSlotWgtState GetSlotState() { return _slotState; }

private:
	class UStratagemWidget* _parentWidget = nullptr;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _stgNameText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _stgStateText;

	UPROPERTY(meta = (BindWidget))
	class UImage* _stgIcon;

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* _widgetSwitcher;

	EStgSlotWgtState _slotState = EStgSlotWgtState::Normal;

	FTimerHandle _startCooldownHandle;
	FTimerHandle _preEndCooldownHandle;
	FTimerHandle _endCooldownHandle;

	bool _isForcedShowing = false;
};
