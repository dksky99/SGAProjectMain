// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../../CGameInstance.h"
#include "PreDeployEntryBase.generated.h"

/**
 * 
 */

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEntryPicked, UPreDeployEntryBase*);

UCLASS()
class SGAPROJECTMAIN_API UPreDeployEntryBase : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeOnInitialized() override;

public:
	UFUNCTION()
	virtual void HandlePick();

	FOnEntryPicked _onPickedEvent;
	
	virtual void InitializeEntry(int32 id);

	virtual void SetEquipped(bool isEquipped);
	void SetSelected(bool isSelected);
	void SetItemID(int32 id) { _itemID = id; }
	void SetEntryIndex(int32 section, int32 row, int32 col);

	int32 GetItemID() { return _itemID; }

	int32 GetSection() { return _section; }
	int32 GetRow() { return _row; }
	int32 GetCol() { return _col; }

protected:
	UPROPERTY(meta = (BindWidget))
	class UButton* _button;

	UPROPERTY(meta = (BindWidget))
	class UBorder* _border; // 버튼 외곽선을 감싸는 보더

	UPROPERTY(meta = (BindWidget))
	class UImage* _itemImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* _equipMark;

	UPROPERTY(EditAnywhere)
	int32 _itemID;

	int32 _section;
	int32 _row;
	int32 _col;

	bool _isSelected = false;
};
