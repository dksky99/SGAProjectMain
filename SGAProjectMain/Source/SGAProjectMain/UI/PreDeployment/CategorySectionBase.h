// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CategorySectionBase.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UCategorySectionBase : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	class UTextBlock* _titleText;
};
