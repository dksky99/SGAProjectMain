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

    void Add(ESampleType type, int32 amount = 1)
    {
        _samples.FindOrAdd(type) += amount;
    }

    void AddSample(const FSampleBundle& other)
    {
        for (const auto& pair : other._samples)
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
    ASampleResources();

	virtual void PickupItem(class AHellDiver* player);

    void SetBundle(FSampleBundle bundle) { _sampleBundle = bundle; }
	
private:
	UPROPERTY(EditAnywhere, Category = "Game/Sample")
    FSampleBundle _sampleBundle;
};
