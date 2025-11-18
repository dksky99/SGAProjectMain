// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MissionResultSlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UMissionResultSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitializeSlot(const struct FMissionResult& missionResult, int32 index);

protected:
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* _titleText;

    // 보상 표시
    UPROPERTY(meta = (BindWidget))
    class UHorizontalBox* _rewardBox;

    UPROPERTY(meta = (BindWidget))
	class UTextBlock* _xpText;

    UPROPERTY(meta = (BindWidget))
	class UTextBlock* _requisitionText;

    // 아이콘 박스
    UPROPERTY(meta = (BindWidget))
    class UHorizontalBox* _iconBox;

    // 시간 퍼센트 표시
    UPROPERTY(meta = (BindWidget))
    class UHorizontalBox* _timeBox;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* _percentText;

    // 이미지 동적 생성용
    UPROPERTY(EditDefaultsOnly, Category = "Game/UI")
    TSubclassOf<class UImage> _iconWidgetClass;
};
