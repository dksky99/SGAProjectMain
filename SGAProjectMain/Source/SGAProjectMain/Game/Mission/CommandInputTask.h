// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TerminalTaskBase.h"
#include "CommandInputTask.generated.h"

/**
 *
 */

UCLASS()
class SGAPROJECTMAIN_API UCommandInputTask : public UTerminalTaskBase
{
	GENERATED_BODY()
	
public:
	virtual void InitializeTask(UUserWidget* terminalWidget) override;
	virtual void StartTask() override;
	virtual void EndTask() override;
	virtual void ResetTask() override;

	virtual void ReceiveInput(FKey key) override;

protected:
	virtual void CheckInputCombo();

	UPROPERTY()
	class UCommandWidget* _commandWidget;

	UPROPERTY(EditAnywhere, Category = "Game/Command")
	TArray<FKey> _command;
	
	TArray<FKey> _playerInputBuffer;
};
