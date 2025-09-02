// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SelectableEntryBase.generated.h"

/**
 * 
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEntryPicked, int32);

UCLASS()
class SGAPROJECTMAIN_API USelectableEntryBase : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeOnInitialized() override;

public:
	UFUNCTION()
	virtual void HandlePick();

	FOnEntryPicked _onPickedEvent;
	
	void SetSelected(bool isSelected);

	int32 GetItemID() { return _itemID; }

protected:
	UPROPERTY(meta = (BindWidget))
	class UButton* _button;

	UPROPERTY(meta = (BindWidget))
	class UBorder* _border; // 버튼 외곽선을 감싸는 보더

	UPROPERTY(EditAnywhere)
	int32 _itemID;

	bool _isSelected = false;
};
