// Fill out your copyright notice in the Description page of Project Settings.


#include "SupplyBox.h"
#include "../../Character/HellDiver/HellDiver.h"

ASupplyBox::ASupplyBox()
{
	_interactableInfo._interactionText = FText::FromString(TEXT("Supply Box"));
	_interactableInfo._type = EInteractableIconType::SupplyBox;
}

void ASupplyBox::PickupItem(AHellDiver* player)
{
	player->RefillAllItem();

	Super::PickupItem(player);
}
