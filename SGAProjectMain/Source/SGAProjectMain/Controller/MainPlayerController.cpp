// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"

#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

#include "Components/Button.h"

AMainPlayerController::AMainPlayerController()
{
}

void AMainPlayerController::PostInitializeComponents()
{
}

void AMainPlayerController::BeginPlay()
{
	//부모의 BeginPlay 호출. 상속구조의 이벤트는 부모의 이벤트를 호출해줘야함
	Super::BeginPlay();

	UEnhancedInputLocalPlayerSubsystem* subSystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

	if (subSystem)
	{
		subSystem->AddMappingContext(_inputMappingContext, 0);
	}
}

void AMainPlayerController::OnPossess(APawn* pawn)
{
}

void AMainPlayerController::OnUnPossess()
{
}
