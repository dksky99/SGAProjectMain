// Fill out your copyright notice in the Description page of Project Settings.


#include "SampleResources.h"
#include "../../Character/HellDiver/HellDiver.h"

void ASampleResources::PickupItem(AHellDiver* player)
{
    player->AddSample(_sampleBundle);

    Super::PickupItem(player);
}
