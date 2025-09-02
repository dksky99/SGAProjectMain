// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SelectableEntryBase.generated.h"

/**
 * 
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEntryPicked, int32);

UCLASS()
class SGAPROJECTMAIN_API USelectableEntryBase : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeOnInitialized() override;

public:
	virtual void HandlePick();

	FOnEntryPicked _onPickedEvent;
	
protected:
	UPROPERTY()
	class UButton* _button;

	UPROPERTY(EditAnywhere)
	int32 _itemID;
};
