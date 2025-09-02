// Fill out your copyright notice in the Description page of Project Settings.


#include "SelectableEntryBase.h"

#include "Components/Button.h"

void USelectableEntryBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	_button->OnClicked.AddDynamic(this, &USelectableEntryBase::HandlePick);
}

void USelectableEntryBase::HandlePick()
{
	if (_onPickedEvent.IsBound())
		_onPickedEvent.Broadcast(_itemID);
}
