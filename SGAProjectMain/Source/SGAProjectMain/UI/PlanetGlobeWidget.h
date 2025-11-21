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
	virtual void NativeConstruct() override;
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

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* _bonusBox1;
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* _bonusBox2;
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* _bonusBox3;
	UPROPERTY()
	TArray<class UVerticalBox*> _bonusBoxes;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _bonusText1;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _bonusText2;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _bonusText3;
	UPROPERTY()
	TArray<class UTextBlock*> _bonusTexts;
};
