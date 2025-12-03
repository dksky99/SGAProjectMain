// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopActor.h"
#include "EnhancedInputSubsystems.h"
#include "../../Character/HellDiver/HellDiver.h"
#include "../../UI/ShopWidgetBase.h"

void AShopActor::Interact(AHellDiver* player)
{
	if (_shopWidgetClass && player)
	{
		APlayerController* PC = Cast<APlayerController>(player->GetController());
		if (!PC) return;

		UShopWidgetBase* shopWidget = CreateWidget<UShopWidgetBase>(PC, _shopWidgetClass);
		if (shopWidget)
		{
			if (auto* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{ // IMC ±³Ã¼
				subsystem->RemoveMappingContext(_gameIMC);
				subsystem->AddMappingContext(_widgetIMC, 10);
			}

			shopWidget->AddToViewport();

			FInputModeGameAndUI mode;
			mode.SetWidgetToFocus(shopWidget->TakeWidget());
			mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(mode);
			PC->bShowMouseCursor = true;
		}
	}
}
