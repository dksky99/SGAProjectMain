// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionTimerWidget.h"
#include "../MainGameMode.h"
#include "Components/TextBlock.h"

void UMissionTimerWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UWorld* World = GetWorld())
    {
        if (auto* GM = Cast<AMainGameMode>(World->GetAuthGameMode()))
        {
            GM->_timerUpdatedEvent.AddUObject(this, &UMissionTimerWidget::HandleTimerUpdated);
        }
    }
}

void UMissionTimerWidget::HandleTimerUpdated(float remainingTime)
{
    int32 remain = FMath::Max(0, remainingTime);

    const int32 minutes = (remain % 3600) / 60;
    const int32 seconds = remain % 60;

    FString TimeString = FString::Printf(TEXT("%02d:%02d"), minutes, seconds);
    _timerText->SetText(FText::FromString(TimeString));
}
