// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Blueprint/UserWidget.h"
#include "TerminalTaskBase.generated.h"

/**
 * 
 */
DECLARE_MULTICAST_DELEGATE(FTaskCompleted);

UCLASS(BlueprintType, Blueprintable)
class SGAPROJECTMAIN_API UTerminalTaskBase : public UDataAsset
{
	GENERATED_BODY()
	
public:
	virtual void InitializeTask(UUserWidget* terminalWidget);
    virtual void StartTask();
    virtual void EndTask();
	virtual void ResetTask();

	virtual void ReceiveInput(FKey key);

	FTaskCompleted _taskCompletedEvent;

	TSubclassOf<UUserWidget> GetTerminalWidgetClass() const { return _terminalWidgetClass; }

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> _terminalWidgetClass;

	UPROPERTY()
	UUserWidget* _terminalWidget;
};
