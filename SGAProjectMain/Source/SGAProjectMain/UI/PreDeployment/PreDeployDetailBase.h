// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "../../CGameInstance.h"
#include "PreDeployDetailBase.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UPreDeployDetailBase : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void SetDetail(int32 id);

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _nameText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _sectionText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _descText;
	
};
