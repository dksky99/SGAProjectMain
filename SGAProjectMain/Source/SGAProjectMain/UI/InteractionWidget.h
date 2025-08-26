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
	void ShowDefaultMark();
	void ShowKeyButtonMark();
	
private:
	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* _iconSwitcher;
};
