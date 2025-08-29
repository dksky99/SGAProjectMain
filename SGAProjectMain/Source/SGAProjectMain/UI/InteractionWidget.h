// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionWidget.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UInteractionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeWidget(struct FInteractableInfo info);
	void ShowDefaultMark();
	void ShowKeyButtonMark();
	
private:
	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* _markSwitcher;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _interactionTxtBlock;

	UPROPERTY(meta = (BindWidget))
	class UImage* _itemIcon;

	UPROPERTY(VisibleAnywhere)
	class UTexture2D* _gunIcon;
	UPROPERTY(VisibleAnywhere)
	class UTexture2D* _sampleIcon;
	UPROPERTY(VisibleAnywhere)
	class UTexture2D* _supplyIcon;
};
