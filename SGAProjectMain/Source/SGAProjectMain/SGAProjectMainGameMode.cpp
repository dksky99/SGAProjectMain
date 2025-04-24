// Copyright Epic Games, Inc. All Rights Reserved.

#include "SGAProjectMainGameMode.h"
#include "SGAProjectMainCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASGAProjectMainGameMode::ASGAProjectMainGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
