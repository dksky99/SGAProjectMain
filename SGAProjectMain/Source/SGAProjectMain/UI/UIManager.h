// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "UIManager.generated.h"

/**
 * 
 */

#define GET_WIDGET(widgetClass, widgetName) \
    Cast<widgetClass>(GetGameInstance()->GetSubsystem<UUIManager>()->GetOrShowPopUp(widgetName))


UCLASS()
class SGAPROJECTMAIN_API UUIManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UUIManager();

	virtual void Initialize(FSubsystemCollectionBase& collection) override;

	bool IsOpen(FString name);

	UUserWidget* GetOrShowPopUp(FString name);
	void ClosePopUp(FString name);
	void ClosePopUp();
	void CloseAll();

private:
	void AddUIConstructor(FString name, FString path);
	TSubclassOf<UUserWidget> GetUIClass(FString name);

private:
	

	TMap<FString, TSubclassOf<UUserWidget>> _widgetMap;

	int32 _zOrder = 1;
	TArray<UUserWidget*> _widgetStack;
};
