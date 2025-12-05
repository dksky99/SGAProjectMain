// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StratagemEtaWidget.generated.h"

UCLASS()
class SGAPROJECTMAIN_API UStratagemEtaWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 남은 시간(초) 표시
	UFUNCTION(BlueprintCallable, Category = "Game/Stratagem/UI")
	void SetEtaSeconds(int32 etaSec);

	// 화면 상에서 위젯 위치 설정
	UFUNCTION(BlueprintCallable, Category = "Game/Stratagem/UI")
	void SetWidgetScreenPosition(const FVector2D& screenPosition);

	// 보이기 / 숨기기
	UFUNCTION(BlueprintCallable, Category = "Game/Stratagem/UI")
	void SetIndicatorVisible(bool isVisible);

protected:
	// UMG에서 바인딩할 텍스트 블록 (예: "ETA: 5s")
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _etaText;


	UPROPERTY(meta = (BindWidget))
	class UBorder* _etaBorder;
};