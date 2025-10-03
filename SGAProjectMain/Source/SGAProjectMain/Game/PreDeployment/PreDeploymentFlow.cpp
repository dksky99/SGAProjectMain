// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeploymentFlow.h"

#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "PreDeploymentState.h"
#include "../../UI/PreDeployment/PreDeployHubWidget.h"
#include "../../Character/PlayerCharacter.h"
#include "../../CGameInstance.h"
#include "../../UI/PreviewStage.h"

void UPreDeploymentFlow::Initialize(APlayerCharacter* player)
{
	_player = player;

    _previewStage = Cast<APreviewStage>(UGameplayStatics::GetActorOfClass(this, APreviewStage::StaticClass()));
}

void UPreDeploymentFlow::EnterFlow()
{
    if (!_player) return;

    if (_previewStage)
    {
		_previewStage->SetPreviewStageFromCharacter(_player);
    }

    APlayerController* PC = Cast<APlayerController>(_player->GetController());
    if (!PC) return;

    UCGameInstance* GI = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
    if (!GI) return;
    _state = GI->GetPreDeployState();
    
    _hubWidgetClass = LoadClass<UPreDeployHubWidget>(nullptr, TEXT("/Game/Blueprints/UI/PreDeployment/BP_PreDeployHub.BP_PreDeployHub_C"));

    if (auto* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
    { // IMC 교체
        subsystem->RemoveMappingContext(_gameIMC);
        subsystem->AddMappingContext(_preDeployWidgetIMC, 10);
    }
    
    if (_hubWidgetClass)
    {
        _hubWidget = CreateWidget<UPreDeployHubWidget>(PC, _hubWidgetClass);
        _hubWidget->InitializeHubWidget(_state);
        _hubWidget->AddToViewport();

        _hubWidget->_OnLaunchEvent.AddDynamic(this, &UPreDeploymentFlow::HandleLaunch);
    }

    FInputModeGameAndUI mode;
    mode.SetWidgetToFocus(_hubWidget->TakeWidget());
    mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    PC->SetInputMode(mode);
    PC->bShowMouseCursor = true;
}

void UPreDeploymentFlow::HandleLaunch()
{
    APlayerController* PC = Cast<APlayerController>(_player->GetController());
    if (!PC) return;
    
    if (auto* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
    { // IMC 교체
        subsystem->RemoveMappingContext(_preDeployWidgetIMC);
        subsystem->AddMappingContext(_gameIMC, 0);
    }
    
    FInputModeGameOnly mode;
    PC->SetInputMode(mode);
    PC->bShowMouseCursor = false;

    UWidgetBlueprintLibrary::SetFocusToGameViewport();

    UGameplayStatics::OpenLevel(this, FName("FirstPersonMap"));
}
