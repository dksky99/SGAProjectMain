// Fill out your copyright notice in the Description page of Project Settings.


#include "SupplyBox.h"
#include "../../Character/HellDiver/HellDiver.h"

void ASupplyBox::PickupItem(AHellDiver* player)
{
	player->RefillAllItem();

	Super::PickupItem(player);
}
