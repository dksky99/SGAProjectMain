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

    void SubtractCurrency(const FPlayerCurrency& other)
    {
        _experience -= other._experience;
        _requisitionSlips -= other._requisitionSlips;
        _medals -= other._medals;
		_samples.SubtractSample(other._samples);
    }

    int32 GetSampleCount(ESampleType type) const
    {
        if (const int32* count = _samples._samples.Find(type))
        {
            return *count;
        }
        return 0;
	}

    bool CanAfford(const FPlayerCurrency& other) const
    {
        if (_requisitionSlips < other._requisitionSlips) return false;
        if (_medals < other._medals)           return false;

        for (const auto& pair : other._samples._samples)
        {
            const ESampleType type = pair.Key;
            const int32 required = pair.Value;

            const int32 have = GetSampleCount(type);
            if (have < required)
                return false;
        }
		return true;
	}
};