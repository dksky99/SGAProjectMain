// Fill out your copyright notice in the Description page of Project Settings.


#include "TerminalConsole.h"

#include "Components/WidgetComponent.h"
#include "../../MainGameMode.h"
#include "../../Character/PlayerCharacter.h"
#include "../../UI/CommandWidget.h"
#include "../../Game/Mission/TerminalTaskBase.h"

ATerminalConsole::ATerminalConsole()
{
	_terminalWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ConsoleWidget"));
	_terminalWidgetComponent->SetupAttachment(RootComponent);
	_terminalWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
}

void ATerminalConsole::BeginPlay()
{
	Super::BeginPlay();

	_terminalWidgetComponent->SetVisibility(true);
	_terminalWidgetComponent->SetWidgetClass(_curTask->GetTerminalWidgetClass());
	_terminalWidgetComponent->InitWidget();
	_terminalWidget = _terminalWidgetComponent->GetUserWidgetObject();

	_curTask->InitializeTask(_terminalWidget); // 임시
	_curTask->_taskCompletedEvent.AddUObject(this, &ATerminalConsole::OnTaskCompleted);
	
	_interactionMark->SetVisibility(false);
}

void ATerminalConsole::Interact(AHellDiver* hellDiver)
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
			return;
		}

		else // 다른 사람이 상호작용을 시도할 경우 작동 x
			return;
	}

	// 아무도 상호작용하고 있지 않을 때
	if (auto player = Cast<APlayerCharacter>(hellDiver))
	{
		player->BeginTerminalInputMode(this);
		_curTask->StartTask(); // 현재 작업 시작
		_terminalWidget->SetVisibility(ESlateVisibility::Visible);
		_interactionMark->SetVisibility(false);
		_player = player;
	}
}

void ATerminalConsole::ReceiveInput(FKey key)
{
	_curTask->ReceiveInput(key);
}

void ATerminalConsole::ShowDefaultMark()
{
	Super::ShowDefaultMark();
	_interactionMark->SetVisibility(_isInteractable);
}

void ATerminalConsole::ShowKeyButtonMark()
{
	Super::ShowKeyButtonMark();
	_interactionMark->SetVisibility(_isInteractable);
}

void ATerminalConsole::SetInteractable(bool isInteractable)
{
	_isInteractable = isInteractable;

	if (_interactionMark)
	{
		_interactionMark->SetVisibility(_isInteractable);
	}
}

void ATerminalConsole::ResetTerminalConsole()
{
	_player->EndTerminalInputMode();
	_curTask->ResetTask();
	_player = nullptr;
	_interactionMark->SetVisibility(_isInteractable);
}

void ATerminalConsole::OnTaskCompleted()
{
	_missionCompletedEvent.Broadcast();

	SetInteractable(false); // 상호작용 불가 상태로 변경
	ResetTerminalConsole();
}
