// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/PanelWidget.h"
#include "SelectableEntryBase.h"
#include "../../Game/PreDeployment/PreDeploymentState.h"
#include "PreDeployPanelBase.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UPreDeployPanelBase : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializePanel(class UPreDeploymentState* state);

	void HandlePicked(int32 itemID);
	
protected:
	UPROPERTY()
	class UPreDeploymentState* _state;

	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* _panel;

	UPROPERTY()
	TArray<class USelectableEntryBase*> _entries;
};
