// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PreDeploymentFlow.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class SGAPROJECTMAIN_API UPreDeploymentFlow : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(class APlayerCharacter* player);

	void EnterFlow();

private:
	UFUNCTION() 
	void HandleLaunch();


	UPROPERTY()
	class UPreDeploymentState* _state;

	UPROPERTY(EditAnywhere, Category = "Game/UI")
	TSubclassOf<UUserWidget> _hubWidgetClass;
	UPROPERTY()
	class UPreDeployHubWidget* _hubWidget;

	UPROPERTY()
	class APlayerCharacter* _player;

	UPROPERTY()
	class APreviewStage* _previewStage;
};
