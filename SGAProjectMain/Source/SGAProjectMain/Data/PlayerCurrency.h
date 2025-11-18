// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Object/Item/SampleResources.h"
#include "PlayerCurrency.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class ECurrencyType : uint8
{
    Experience,
    RequisitionSlips,
    Medals
};

USTRUCT(BlueprintType)
struct SGAPROJECTMAIN_API FPlayerCurrency
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
    int32 _experience = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
    int32 _requisitionSlips = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
    int32 _medals = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
    FSampleBundle _samples;

    // ÇÔ¼öµé
    void Add(ECurrencyType type, int32 amount = 1)
    {
        switch (type)
        {
        case ECurrencyType::Experience:
            _experience += amount;
            break;
        case ECurrencyType::RequisitionSlips:
            _requisitionSlips += amount;
            break;
        case ECurrencyType::Medals:
            _medals += amount;
            break;
        }
    }

    void AddCurrency(const FPlayerCurrency& other)
    {
        _experience += other._experience;
        _requisitionSlips += other._requisitionSlips;
        _medals += other._medals;
        _samples.AddSample(other._samples);
	}

    void AddSample(const FSampleBundle& sample)
    {
        _samples.AddSample(sample);
    }

    int32 Get(ECurrencyType type)
    {
        switch (type)
        {
        case ECurrencyType::Experience:
            return _experience;
        case ECurrencyType::RequisitionSlips:
            return _requisitionSlips;
        case ECurrencyType::Medals:
            return _medals;
        }
        return 0;
    }
};