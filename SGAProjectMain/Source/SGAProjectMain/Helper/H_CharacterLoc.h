// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class SGAPROJECTMAIN_API H_CharacterLoc
{
public:
	static void SetCharacterToGround(ACharacter* player, USceneComponent* anchor, UWorld* world);
};
