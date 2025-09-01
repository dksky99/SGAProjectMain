// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeploymentFlow.h"

#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "PreDeploymentState.h"
#include "../../UI/PreDeployment/PreDeployHubWidget.h"
#include "../../Character/PlayerCharacter.h"

void UPreDeploymentFlow::Initialize(APlayerCharacter* player)
{
	_player = player;
}

void UPreDeploymentFlow::EnterFlow()
{
    if (!_player) return;

    if (!_state)
        _state = NewObject<UPreDeploymentState>(this);
    
    _hubWidgetClass = LoadClass<UPreDeployHubWidget>(nullptr, TEXT("/Game/Blueprints/UI/PreDeployment/BP_PreDeployHub.BP_PreDeployHub_C"));

    if (_hubWidgetClass)
    {
        _hubWidget = CreateWidget<UPreDeployHubWidget>(GetWorld(), _hubWidgetClass);
        _hubWidget->InitializeWidget(_state);
        _hubWidget->AddToViewport();

        _hubWidget->_OnLaunchEvent.AddDynamic(this, &UPreDeploymentFlow::HandleLaunch);
    }
    
    APlayerController* PC = Cast<APlayerController>(_player->GetController());
    PC->bShowMouseCursor = true;
}

void UPreDeploymentFlow::HandleLaunch()
{
    APlayerController* PC = Cast<APlayerController>(_player->GetController());
    PC->bShowMouseCursor = false;

    UWidgetBlueprintLibrary::SetFocusToGameViewport();

    UGameplayStatics::OpenLevel(this, FName("FirstPersonMap"));
}
