// Fill out your copyright notice in the Description page of Project Settings.


#include "DropPlaneBeacon.h"

#include "TerminalConsole.h"
#include "Engine/DamageEvents.h"
#include "../../MainGameMode.h"

ADropPlaneBeacon::ADropPlaneBeacon()
{
    _mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = _mesh;
}

void ADropPlaneBeacon::OnCommandCompleted()
{
    AMainGameMode* GM = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this));
    if (GM)
    {
        GM->CallEscapePlane();
    }
}

void ADropPlaneBeacon::SetInteractable(bool isInteractable)
{
    if (_terminalChild)
    {
        if (auto terminalConsole = Cast<ATerminalConsole>(_terminalChild->GetChildActor()))
        {
			terminalConsole->SetInteractable(isInteractable);
        }
    }
}
