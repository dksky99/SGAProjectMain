// Fill out your copyright notice in the Description page of Project Settings.


#include "TerminalConsole.h"

#include "Components/WidgetComponent.h"
#include "../../MainGameMode.h"
#include "../../Character/PlayerCharacter.h"
#include "../../UI/CommandWidget.h"

ATerminalConsole::ATerminalConsole()
{
	_terminalWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ConsoleWidget"));
	_terminalWidgetComponent->SetupAttachment(RootComponent);

	_interactionMark = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionMark"));
	_interactionMark->SetupAttachment(RootComponent);

	_terminalWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	_interactionMark->SetWidgetSpace(EWidgetSpace::Screen);
}

void ATerminalConsole::BeginPlay()
{
	Super::BeginPlay();

	auto widget = _terminalWidgetComponent->GetUserWidgetObject();
	if (auto commandWidget = Cast<UCommandWidget>(widget))
		_terminalWidget = commandWidget;

	_terminalWidget->InitializeSlot(_command);
	_terminalWidgetComponent->SetVisibility(true);
	_terminalWidget->SetVisibility(ESlateVisibility::Hidden);

	_interactionMark->SetVisibility(_isInteractable); // 상호작용 가능할 때만 표시
}

void ATerminalConsole::PickupItem(AHellDiver* hellDiver)
{
	if (!_isInteractable) return;

	// 터미널이 누군가와 이미 상호작용 중일 때
	if (_player)
	{
		// 상호작용을 시도한 사람이 현재 시도한 사람과 같을 경우
		if (_player == hellDiver)
		{
			// 상호작용 해제
			ResetTerminalConsole();
			_terminalWidget->SetVisibility(ESlateVisibility::Hidden);
		}

		else // 다른 사람이 상호작용을 시도할 경우 작동 x
			return;
	}

	// 아무도 상호작용하고 있지 않을 때
	if (auto player = Cast<APlayerCharacter>(hellDiver))
	{
		player->BeginTerminalInputMode(this);
		_terminalWidget->SetVisibility(ESlateVisibility::Visible);
		_interactionMark->SetVisibility(false);
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

void ATerminalConsole::SetInteractable(bool isInteractable)
{
	_isInteractable = isInteractable;

	if (_interactionMark)
	{
		_interactionMark->SetVisibility(_isInteractable);
	}
}

void ATerminalConsole::CheckInputCombo()
{
	// 완전 일치 → 장비
	if (_playerInputBuffer == _command)
	{
		_commandSuccess.Broadcast();

		SetInteractable(false); // 상호작용 불가 상태로 변경
		ResetTerminalConsole();
		_terminalWidget->OnCompleted();

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
		ResetTerminalConsole();
		_terminalWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		_terminalWidget->UpdateSlot(_playerInputBuffer.Num());
	}
}

void ATerminalConsole::ResetTerminalConsole()
{
	_player->EndTerminalInputMode();
	_playerInputBuffer.Empty();
	_player = nullptr;
	_terminalWidget->ResetSlot();
	_interactionMark->SetVisibility(_isInteractable);
}
