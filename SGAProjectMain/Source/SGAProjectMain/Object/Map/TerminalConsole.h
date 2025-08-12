// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Item/ItemBase.h"
#include "TerminalConsole.generated.h"

/**
 * 
 */
DECLARE_MULTICAST_DELEGATE(FCommandCompleted);

UCLASS()
class SGAPROJECTMAIN_API ATerminalConsole : public AItemBase
{
	GENERATED_BODY()
	
public:
	ATerminalConsole();

	void BeginPlay();

	virtual void PickupItem(class AHellDiver* hellDiver);
	void ReceiveInput(FKey key);

	FCommandCompleted _commandSuccess;

	void SetInteractable(bool isInteractable) { _isInteractable = isInteractable; }

protected:
	void CheckInputCombo();
	void ResetInput();

protected:
	UPROPERTY(EditAnywhere, Category = "Game/Command")
	class UWidgetComponent* _terminalWidgetComponent;

	UPROPERTY()
	class UCommandWidget* _terminalWidget;

	UPROPERTY()
	class APlayerCharacter* _player;

	UPROPERTY(EditAnywhere, Category = "Game/Command")
	TArray<FKey> _command;

	TArray<FKey> _playerInputBuffer;

	UPROPERTY(EditAnywhere, Category = "Game/Console")
	bool _isInteractable = true;
};
