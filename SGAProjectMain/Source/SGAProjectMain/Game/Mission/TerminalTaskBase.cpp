// Fill out your copyright notice in the Description page of Project Settings.


#include "TerminalTaskBase.h"

void UTerminalTaskBase::InitializeTask(UUserWidget* terminalWidget)
{
	if (!terminalWidget)
		return;

	_terminalWidget = terminalWidget;
	// ±âº»ÀûÀ¸·Î À§Á¬À» ¼û±è
	if (_terminalWidget)
	{
		_terminalWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UTerminalTaskBase::StartTask()
{
	_terminalWidget->SetVisibility(ESlateVisibility::Visible);
}	

void UTerminalTaskBase::EndTask()
{
	_taskCompletedEvent.Broadcast();
}

void UTerminalTaskBase::ResetTask()
{

}

void UTerminalTaskBase::ReceiveInput(FKey key)
{
	
}
