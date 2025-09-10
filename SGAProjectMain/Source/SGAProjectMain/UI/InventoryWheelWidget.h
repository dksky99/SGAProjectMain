// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWheelWidget.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UInventoryWheelWidget : public UUserWidget
{
	GENERATED_BODY()
	
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
	int32 GetCurIndex() { return _curIndex; }

protected:
	float centerRadius = 250.f; // 중앙 취소 영역 반지름
	int32 _curIndex = -1;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _indexText;
};
