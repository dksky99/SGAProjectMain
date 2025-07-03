// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MiniMapWidget.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UMiniMapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ResetMap();

	UFUNCTION()
	void SetCursorText(FVector sceneCapturerToCursor, FVector playerToCursor, float halfWidth);

	UFUNCTION()
	void SetPingImage(FVector sceneCapturerToPing, float halfWidth); // 위치 표시
	UFUNCTION()
	void ShowPingImage(bool needToShow); // 아이콘 온오프


	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* _canvasPanel;

	UPROPERTY(meta = (BindWidget))
	class UImage* _mapImage;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _cursorPosText;

	UPROPERTY(meta = (BindWidget))
	class UImage* _pingImage;
};
