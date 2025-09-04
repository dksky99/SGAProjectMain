// Fill out your copyright notice in the Description page of Project Settings.


#include "TerminalConsole.h"

#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
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

	_lookAt = CreateDefaultSubobject<USceneComponent>(TEXT("LookAt"));
	_lookAt->SetupAttachment(RootComponent);

	_playerAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("PlayerAnchor"));
	_playerAnchor->SetupAttachment(RootComponent);
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
	// 플레이어 위치 세팅
	FVector playerLoc = _playerAnchor->GetComponentLocation();

	UCapsuleComponent* Cap = _player->GetCapsuleComponent();
	const float HalfH = Cap->GetScaledCapsuleHalfHeight();

	// 1) 앵커 위→아래로 레이 쏴서 바닥 찾기
	const FVector Anchor = _playerAnchor->GetComponentLocation();
	const FVector Start = Anchor + FVector(0, 0, 200.f);
	const FVector End = Anchor - FVector(0, 0, 5000.f);

	FHitResult Hit;
	FCollisionQueryParams Q(SCENE_QUERY_STAT(SnapToGround), false);
	Q.AddIgnoredActor(_player);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Q);

	// 2) 맞으면 그 지점 + HalfHeight 로 순간이동(바닥에 ‘착’)
	FVector NewLoc = Anchor;
	if (bHit) NewLoc.Z = Hit.ImpactPoint.Z + HalfH;
	_player->SetActorLocation(NewLoc, false);     // false : 충돌로 밀려나지 않음

	// 카메라 위치 세팅
	_cutInCam = GetWorld()->SpawnActor<ACameraActor>();
	_cutInCam->GetCameraComponent()->bConstrainAspectRatio = false;

	FVector camLoc = _camAnchor->GetComponentLocation();
	FRotator camRot = UKismetMathLibrary::FindLookAtRotation(camLoc, _lookAt->GetComponentLocation()); // 화면을 바라보게

	_cutInCam->SetActorLocation(camLoc);
	_cutInCam->SetActorRotation(camRot);

	// 뷰 전환
	APlayerController* PC = Cast<APlayerController>(_player->GetController());
	_playerViewTarget = PC->GetViewTarget();
	PC->SetViewTargetWithBlend(_cutInCam, 0.85f, EViewTargetBlendFunction::VTBlend_Cubic);

	// 현재 작업 시작
	_curTask->StartTask(); 

	// 위젯 visibility 관리
	_interactionMark->SetVisibility(false);
	_terminalWidget->SetVisibility(ESlateVisibility::Visible);
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
