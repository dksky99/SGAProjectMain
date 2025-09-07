// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../../CGameInstance.h"
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

	UFUNCTION()
	virtual void HandlePick();

public:
	FOnEntryPicked _onPickedEvent;
	
	virtual void InitializeEntry(int32 id);
	void SetSelected(bool isSelected);
	void SetItemID(int32 id) { _itemID = id; }

	int32 GetItemID() { return _itemID; }

protected:
	UPROPERTY(meta = (BindWidget))
	class UButton* _button;

	UPROPERTY(meta = (BindWidget))
	class UBorder* _border; // 버튼 외곽선을 감싸는 보더

	UPROPERTY(meta = (BindWidget))
	class UImage* _itemImage;

	UPROPERTY(EditAnywhere)
	int32 _itemID;

	bool _isSelected = false;
};
