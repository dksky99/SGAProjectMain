// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Object/Item/SampleResources.h"
#include "SampleWidget.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API USampleWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetSampleCount(const FSampleBundle& sampleBundle);

private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _commonSampleText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _rareSampleText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _superSampleText;
};
