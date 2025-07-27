// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "SampleResources.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class ESampleType : uint8
{
	Common,
	Rare,
	Super
};

USTRUCT(BlueprintType)
struct FSampleBundle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<ESampleType, int32> _samples;

    void Add(ESampleType Type, int32 Amount = 1)
    {
        _samples.FindOrAdd(Type) += Amount;
    }

    void AddSample(const FSampleBundle& Other)
    {
        for (const auto& pair : Other._samples)
        {
            Add(pair.Key, pair.Value);
        }
    }

    bool IsEmpty() const
    {
        return _samples.IsEmpty();
    }

    void Clear()
    {
        _samples.Empty();
    }
};

UCLASS()
class SGAPROJECTMAIN_API ASampleResources : public AItemBase
{
	GENERATED_BODY()

public:
	virtual void PickupItem(class AHellDiver* player);

    void SetBundle(FSampleBundle bundle) { _sampleBundle = bundle; }
	
private:
	UPROPERTY(EditAnywhere, Category = "Game/Sample")
    FSampleBundle _sampleBundle;
};
