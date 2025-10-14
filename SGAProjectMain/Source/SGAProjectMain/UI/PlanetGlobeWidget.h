// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlanetGlobeWidget.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UPlanetGlobeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowOperation(bool visibility, class APlanetOperationSite* site = nullptr);
	void ShowObjection(bool visibility, class APlanetObjectiveIcon* icon = nullptr);

protected:
	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* _widgetSwitcher;
	UPROPERTY(meta=(BindWidget))
	class UBorder* _operationBox;
	UPROPERTY(meta = (BindWidget))
	class UBorder* _objectiveBox;
};
