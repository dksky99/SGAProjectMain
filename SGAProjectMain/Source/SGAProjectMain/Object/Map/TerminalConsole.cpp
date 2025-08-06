// Fill out your copyright notice in the Description page of Project Settings.


#include "TerminalConsole.h"

#include "Components/WidgetComponent.h"
#include "../../Character/PlayerCharacter.h"
#include "../../UI/CommandWidget.h"

ATerminalConsole::ATerminalConsole()
{
	_terminalWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ConsoleWidget"));
	_terminalWidgetComponent->SetupAttachment(RootComponent);

	_terminalWidgetComponent->SetWidgetSpace(EWidgetSpace::World);


}

void ATerminalConsole::BeginPlay()
{
	Super::BeginPlay();

	auto widget = _terminalWidgetComponent->GetUserWidgetObject();
	if (auto commandWidget = Cast<UCommandWidget>(widget))
		_terminalWidget = commandWidget;

	_terminalWidget->InitializeSlot(_command);
	_terminalWidgetComponent->SetVisibility(false);
}

void ATerminalConsole::PickupItem(AHellDiver* hellDiver)
{
	// 터미널이 누군가와 이미 상호작용 중일 때
	if (_player)
	{
		// 상호작용을 시도한 사람이 현재 시도한 사람과 같을 경우
		if (_player == hellDiver)
		{
			// 상호작용 해제
			_player->EndTerminalInputMode();
			_terminalWidgetComponent->SetVisibility(false);
			_player = nullptr;
		}

		else // 다른 사람이 상호작용을 시도할 경우 작동 x
			return;
	}

	// 아무도 상호작용하고 있지 않을 때
	if (auto player = Cast<APlayerCharacter>(hellDiver))
	{
		player->BeginTerminalInputMode(this);
		_terminalWidgetComponent->SetVisibility(true);
		_player = player;
	}

	// 누군가와 상호작용 중일 때

	_playerInputBuffer.Empty();
}

void ATerminalConsole::ReceiveInput(FKey key)
{
	_playerInputBuffer.Add(key);
	CheckInputCombo();
}

void ATerminalConsole::CheckInputCombo()
{
	// 완전 일치 → 장비
	if (_playerInputBuffer == _command)
	{
		//Operating머시기

		ResetInput();

		_terminalWidgetComponent->SetVisibility(false); // test
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
		ResetInput();
	}
	else
	{
		_terminalWidget->UpdateSlot(_playerInputBuffer.Num());
	}
}

void ATerminalConsole::ResetInput()
{
	_player->EndTerminalInputMode();
	_playerInputBuffer.Empty();
	_terminalWidget->ResetSlot();
}
