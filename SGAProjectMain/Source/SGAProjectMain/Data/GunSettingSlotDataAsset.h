// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GunSettingSlotDataAsset.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class ESlotCategory : uint8
{
    FireMode,
    TacticalLight,
    Scope
};

UCLASS()
class SGAPROJECTMAIN_API UGunSettingSlotDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    ESlotCategory _category;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName _optionKey; // "Auto", "Burst", "Off"...

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText _displayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UTexture2D* _icon;
};
