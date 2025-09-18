// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeploymentFlow.h"

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

    UCGameInstance* GI = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
    if (!GI) return;
    _state = GI->GetPreDeployState();
    
    _hubWidgetClass = LoadClass<UPreDeployHubWidget>(nullptr, TEXT("/Game/Blueprints/UI/PreDeployment/BP_PreDeployHub.BP_PreDeployHub_C"));

    if (_hubWidgetClass)
    {
        _hubWidget = CreateWidget<UPreDeployHubWidget>(GetWorld(), _hubWidgetClass);
        _hubWidget->InitializeHubWidget(_state);
        _hubWidget->AddToViewport();

        _hubWidget->_OnLaunchEvent.AddDynamic(this, &UPreDeploymentFlow::HandleLaunch);
    }
    
    APlayerController* PC = Cast<APlayerController>(_player->GetController());
    FInputModeUIOnly mode;
    mode.SetWidgetToFocus(_hubWidget->TakeWidget());
    mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    PC->SetInputMode(mode);
    PC->bShowMouseCursor = true;
}

void UPreDeploymentFlow::HandleLaunch()
{
    APlayerController* PC = Cast<APlayerController>(_player->GetController());
    FInputModeGameOnly mode;
    PC->SetInputMode(mode);
    PC->bShowMouseCursor = false;

    UWidgetBlueprintLibrary::SetFocusToGameViewport();

    UGameplayStatics::OpenLevel(this, FName("FirstPersonMap"));
}
