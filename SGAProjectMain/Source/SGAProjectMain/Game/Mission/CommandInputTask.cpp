// Fill out your copyright notice in the Description page of Project Settings.


#include "CommandInputTask.h"

#include "../../UI/CommandWidget.h"

void UCommandInputTask::InitializeTask(UUserWidget* terminalWidget)
{
	Super::InitializeTask(terminalWidget);

	if (!terminalWidget)
		return;

	if (auto commandWidget = Cast<UCommandWidget>(terminalWidget))
	{
		_commandWidget = commandWidget;
		_commandWidget->InitializeSlot(_command);
	}
		
}

void UCommandInputTask::StartTask()
{
	Super::StartTask();

	_playerInputBuffer.Empty();
}

void UCommandInputTask::EndTask()
{
	Super::EndTask();
}

void UCommandInputTask::ResetTask()
{
	_playerInputBuffer.Empty();
	_commandWidget->ResetSlot();
}

void UCommandInputTask::ReceiveInput(FKey key)
{
	_playerInputBuffer.Add(key);
	CheckInputCombo();
}

void UCommandInputTask::CheckInputCombo()
{
	// 완전 일치 → 장비
	if (_playerInputBuffer == _command)
	{
		EndTask();
		_commandWidget->OnCompleted();
		return;
	}
	
	// 입력이 끝나지 않은 동안에는
	bool bPrefixMatch = true;
	if (_playerInputBuffer.Num() <= _command.Num())
	{
		for (int32 i = 0; i < _playerInputBuffer.Num(); ++i)
		{
			if (_playerInputBuffer[i] != _command[i]) // 입력이 틀림
			{
				bPrefixMatch = false;
				break;
			}
		}
	}
	if (!bPrefixMatch)
	{
		ResetTask();
	}
	else
	{
		_commandWidget->UpdateSlot(_playerInputBuffer.Num());
	}
}
