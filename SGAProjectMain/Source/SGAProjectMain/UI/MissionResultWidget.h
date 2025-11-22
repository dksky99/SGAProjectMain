// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../MainGameMode.h"
#include "MissionResultWidget.generated.h"

/**
 * 
 */
DECLARE_DELEGATE(FOnRewardFlowFinished)

UCLASS()
class SGAPROJECTMAIN_API UMissionResultWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	void InitializeWidget(const struct FMissionResult& missionResult);

	void ShowRewardsSequentially(); // 순차적으로 보상 표시
	void HideRewardsSequentially(); // 순차적으로 보상 숨기기
	void ShowTotalRewards();

	void ShowOperationStatus();

	void ShowPayoutSummary();

	FOnRewardFlowFinished _rewardFlowFinishedEvent;

protected:
	// 보상 패널
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* _rewardPanel;

	UPROPERTY(meta = (BindWidget))
	class UMissionResultSlotWidget* _mainObjSlot;
	UPROPERTY(meta = (BindWidget))
	class UMissionResultSlotWidget* _optionalObjSlot;
	UPROPERTY(meta = (BindWidget))
	class UMissionResultSlotWidget* _extractedSlot;
	UPROPERTY(meta = (BindWidget))
	class UMissionResultSlotWidget* _timeSlot;

	UPROPERTY(meta = (BindWidget))
	class UBorder* _xpBox;
	UPROPERTY(meta = (BindWidget))
	class UBorder* _requisitionBox;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _totalXpText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _totalRequisitionText;

	TArray<class UMissionResultSlotWidget*> _rewardSlots;

	// 임무 패널
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* _operationPanel;

	UPROPERTY(meta = (BindWidget))
	class UImage* _missionIcon1;
	UPROPERTY(meta = (BindWidget))
	class UImage* _missionIcon2;
	UPROPERTY(meta = (BindWidget))
	class UImage* _missionIcon3;

	UPROPERTY(meta = (BindWidget))
	class UImage* _missionCheck1;
	UPROPERTY(meta = (BindWidget))
	class UImage* _missionCheck2;
	UPROPERTY(meta = (BindWidget))
	class UImage* _missionCheck3;

	TArray<UImage*> _missionIcons;
	TArray<UImage*> _missionChecks;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* _bonusBox1;
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* _bonusBox2;
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* _bonusBox3;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _bonusText1;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _bonusText2;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _bonusText3;

	TArray<class UVerticalBox*> _bonusBoxes;
	TArray<class UTextBlock*> _bonusTexts;

	// 전체 획득물 패널
	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* _payoutPanel;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _commonSampleText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _rareSampleText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _superSampleText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _finalXpText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _medalText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _finalRequisitionText;


	UPROPERTY()
	FMissionResult _missionResult;

	FTimerHandle _rewardTimerHandle;
	int32 _curRewardIndex = 0;
};
