// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerStatusWidget.h"
#include "../CGameInstance.h"
#include "../CSaveGame.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UPlayerStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UCGameInstance* GI = Cast<UCGameInstance>(GetGameInstance());
	if (!GI) return;

	const int32 playerLevel = GI->GetCurrentSave()->GetPlayerLevel();
	_playerLevelText->SetText(FText::AsNumber(playerLevel));

	const int32 curExp = GI->GetCurrentSave()->GetPlayerExperience();
	const int32 expToNextLevel = GI->GetExpToNextLevel(playerLevel);

	_playerExpText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), curExp, expToNextLevel)));
	
	float expPercent = 0.f;
	if (expToNextLevel > 0)
		expPercent = static_cast<float>(curExp) / static_cast<float>(expToNextLevel);

	_playerExpProgressBar->SetPercent(expPercent);
}
