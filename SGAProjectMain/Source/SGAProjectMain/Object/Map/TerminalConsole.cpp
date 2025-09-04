// Fill out your copyright notice in the Description page of Project Settings.


#include "TerminalConsole.h"

#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraActor.h"
#include "Kismet/KismetMathLibrary.h"

#include "../../MainGameMode.h"
#include "../../Character/PlayerCharacter.h"
#include "../../UI/CommandWidget.h"
#include "../../Game/Mission/TerminalTaskBase.h"

ATerminalConsole::ATerminalConsole()
{
	_terminalWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ConsoleWidget"));
	_terminalWidgetComponent->SetupAttachment(RootComponent);
	_terminalWidgetComponent->SetWidgetSpace(EWidgetSpace::World);

	_camAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("CamAnchor"));
	_camAnchor->SetupAttachment(RootComponent);

	_camLookAt = CreateDefaultSubobject<USceneComponent>(TEXT("CamLookAt"));
	_camLookAt->SetupAttachment(RootComponent);
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
		_player = player;
		ActivateTerminalConsole();
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
	if (_player)
		return;

	Super::ShowKeyButtonMark();
	_interactionMark->SetVisibility(_isInteractable);
}

void ATerminalConsole::SetInteractable(bool isInteractable)
{
	_isInteractable = isInteractable;
}

void ATerminalConsole::ActivateTerminalConsole()
{
	_player->BeginTerminalInputMode(this);
	_curTask->StartTask(); // 현재 작업 시작
	_interactionMark->SetVisibility(false);
	_terminalWidget->SetVisibility(ESlateVisibility::Visible);

	_cutInCam = GetWorld()->SpawnActor<ACameraActor>();
	_cutInCam->GetCameraComponent()->bConstrainAspectRatio = false;

	// 카메라 위치 세팅
	const FVector camLoc = _camAnchor->GetComponentLocation();
	const FRotator camRot = UKismetMathLibrary::FindLookAtRotation(camLoc, _camLookAt->GetComponentLocation()); // 화면을 바라보게

	_cutInCam->SetActorLocation(camLoc);
	_cutInCam->SetActorRotation(camRot);

	// 뷰 전환
	APlayerController* PC = Cast<APlayerController>(_player->GetController());
	_playerViewTarget = PC->GetViewTarget();
	PC->SetViewTargetWithBlend(_cutInCam, 0.85f, EViewTargetBlendFunction::VTBlend_Cubic);
}

void ATerminalConsole::ResetTerminalConsole()
{
	_player->EndTerminalInputMode();
	_curTask->ResetTask();

	APlayerController* PC = Cast<APlayerController>(_player->GetController());
	PC->SetViewTargetWithBlend(_playerViewTarget, 0.85f, EViewTargetBlendFunction::VTBlend_Cubic);
	_cutInCam->Destroy();

	_player = nullptr;
	_interactionMark->SetVisibility(_isInteractable);
}

void ATerminalConsole::OnTaskCompleted()
{
	if (_missionCompletedEvent.IsBound())
		_missionCompletedEvent.Broadcast();

	SetInteractable(false); // 상호작용 불가 상태로 변경
	ResetTerminalConsole();
}
