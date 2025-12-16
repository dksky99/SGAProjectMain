// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerStatusWidget.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UPlayerStatusWidget : public UUserWidget
{
	GENERATED_BODY()

	void NativeConstruct() override;
	
protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _playerNameText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _playerLevelText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _playerExpText;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* _playerExpProgressBar;
};
