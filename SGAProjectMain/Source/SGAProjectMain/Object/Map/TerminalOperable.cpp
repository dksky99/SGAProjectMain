// Fill out your copyright notice in the Description page of Project Settings.


#include "TerminalOperable.h"
#include "TerminalConsole.h"
#include "Components/ChildActorComponent.h"

// Sets default values
ATerminalOperable::ATerminalOperable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_terminalChild = CreateDefaultSubobject<UChildActorComponent>(TEXT("TerminalChild"));
	_terminalChild->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ATerminalOperable::BeginPlay()
{
	Super::BeginPlay();

	if (_terminalChild)
	{
		if (auto terminalConsole = Cast<ATerminalConsole>(_terminalChild->GetChildActor()))
		{
			terminalConsole->_missionCompletedEvent.AddUObject(this, &ATerminalOperable::OnCommandCompleted);
		}
	}
}

// Called every frame
void ATerminalOperable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATerminalOperable::OnCommandCompleted()
{
}

