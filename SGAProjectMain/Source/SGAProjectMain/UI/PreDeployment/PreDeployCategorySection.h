// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PreDeployEntryBase.h"
#include "PreDeployCategorySection.generated.h"

/**
 * 
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEntrySpawned, UPreDeployEntryBase*);

UCLASS()
class SGAPROJECTMAIN_API UPreDeployCategorySection : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeSection(FText title, const TArray<int32> ids, int32 sectionIndex);

	FOnEntrySpawned _onEntrySpawnedEvent;

	TArray<UPreDeployEntryBase*> GetEntries() { return _entries; }
	int32 GetEntryNumPerRow() { return _entryRowWidth; }

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
	int32 _entryRowWidth = 2;
};
