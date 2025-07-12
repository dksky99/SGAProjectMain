// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CompassWidget.generated.h"

/**
 * 
 */

struct FCompassDir
{
	float angle;
	class UTextBlock* textBlock;
};

UCLASS()
class SGAPROJECTMAIN_API UCompassWidget : public UUserWidget
{
	GENERATED_BODY()
	
	void NativeConstruct() override;
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void UpdateWidgetPos(UWidget* widget, float delta);

public:
	void ShowPingImage(bool needToShow);
	void SetPingLocation(FVector pingLoc) { _pingLocation = pingLoc; }

private:
	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* _canvasPanel;

	UPROPERTY(meta = (BindWidget))
	class UImage* _compassImage;

	// 각도 표시
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _angleText;

	// 방향 표시
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _northText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _eastText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _southText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _westText;

	// 핑 표시
	UPROPERTY(meta = (BindWidget))
	class UImage* _pingImage;

	UPROPERTY()
	UMaterialInstanceDynamic* _compassMat;

	UPROPERTY()
	APlayerController* _pc;

	TArray<FCompassDir> _directionMarks;
	
	bool _isPingActive = false;
	FVector _pingLocation;
};
