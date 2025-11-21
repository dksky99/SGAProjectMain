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
	void EnterOperationMode();
	void EnterMissionMode();
	void ShowOperation(bool visibility, class APlanetOperationSite* site = nullptr);
	void ShowMission(bool visibility, class APlanetMissionIcon* icon = nullptr);

protected:
	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* _widgetSwitcher;
	UPROPERTY(meta=(BindWidget))
	class UBorder* _operationBox;
	UPROPERTY(meta = (BindWidget))
	class UBorder* _missionBox;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _operationNameText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _operationDescText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _missionNameText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _missionDescText;
	UPROPERTY(meta = (BindWidget))
	class UImage* _missionIcon;
};
