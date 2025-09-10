// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PreDeployEntryBase.h"
#include "PreDeployCategorySection.generated.h"

/**
 * 
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSectionPicked, int32, UPreDeployEntryBase*);

UCLASS()
class SGAPROJECTMAIN_API UPreDeployCategorySection : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeSection(FText title, const TArray<int32> ids);

	void HandlePicked(int32 itemID); // 엔트리에서 전달받은 아이디

	FOnSectionPicked _onSectionPickedEvent;

protected:
	UPROPERTY(EditAnywhere, Category = "Game/UI")
	TSubclassOf<class UPreDeployEntryBase> _entryClass;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _titleText;

	UPROPERTY(meta = (BindWidget))
	class UUniformGridPanel* _entryPanel;

	UPROPERTY()
	TArray<UPreDeployEntryBase*> _entries;

	UPROPERTY(EditAnywhere, Category = "Game/UI")
	int32 _entriesPerRow = 2;
};
