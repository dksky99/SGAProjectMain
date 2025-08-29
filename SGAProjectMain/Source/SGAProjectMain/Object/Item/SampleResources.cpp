// Fill out your copyright notice in the Description page of Project Settings.


#include "SampleResources.h"
#include "../../Character/HellDiver/HellDiver.h"

ASampleResources::ASampleResources()
{
    _interactableInfo._interactionText = FText::FromString(TEXT("Sample"));
    _interactableInfo._type = EInteractableIconType::Sample;
}

void ASampleResources::PickupItem(AHellDiver* player)
{
    player->AddSample(_sampleBundle);

    Super::PickupItem(player);
}
